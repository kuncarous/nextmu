#include "stdafx.h"
#include "mu_environment_particles.h"
#include "mu_resourcesmanager.h"
#include "mu_threadsmanager.h"
#include "t_particles.h"
#include "t_particle_entity.h"

using namespace TParticle;
constexpr mu_boolean UseMultithread = true;

const mu_boolean NParticles::Initialize()
{
#if NEXTMU_COMPRESSED_PARTICLES == 1
	RenderBuffer.Layout
		.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Int16, true, true)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true, true)
		.end();
#else
	RenderBuffer.Layout
		.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.end();
#endif
	RenderBuffer.VertexBuffer = bgfx::createDynamicVertexBuffer(TParticle::MaxRenderCount * 4, RenderBuffer.Layout);
	if (bgfx::isValid(RenderBuffer.VertexBuffer) == false)
	{
		return false;
	}

	RenderBuffer.IndexBuffer = bgfx::createDynamicIndexBuffer(TParticle::MaxRenderCount * 6, BGFX_BUFFER_INDEX32);
	if (bgfx::isValid(RenderBuffer.IndexBuffer) == false)
	{
		return false;
	}

	RenderBuffer.Projection = bgfx::createUniform("u_projection", bgfx::UniformType::Mat4);
	if (bgfx::isValid(RenderBuffer.Projection) == false)
	{
		return false;
	}

	RenderBuffer.TextureSampler = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
	if (bgfx::isValid(RenderBuffer.TextureSampler) == false)
	{
		return false;
	}

	RenderBuffer.Program = MUResourcesManager::GetProgram("particle");
	if (bgfx::isValid(RenderBuffer.Program) == false)
	{
		return false;
	}

	return true;
}

void NParticles::Destroy()
{
	if (bgfx::isValid(RenderBuffer.VertexBuffer))
	{
		bgfx::destroy(RenderBuffer.VertexBuffer);
		RenderBuffer.VertexBuffer = BGFX_INVALID_HANDLE;
	}

	if (bgfx::isValid(RenderBuffer.IndexBuffer))
	{
		bgfx::destroy(RenderBuffer.IndexBuffer);
		RenderBuffer.IndexBuffer = BGFX_INVALID_HANDLE;
	}

	if (bgfx::isValid(RenderBuffer.Projection))
	{
		bgfx::destroy(RenderBuffer.Projection);
		RenderBuffer.Projection = BGFX_INVALID_HANDLE;
	}

	if (bgfx::isValid(RenderBuffer.TextureSampler))
	{
		bgfx::destroy(RenderBuffer.TextureSampler);
		RenderBuffer.TextureSampler = BGFX_INVALID_HANDLE;
	}
}

void NParticles::Create(const NParticleData &data)
{
	PendingToCreate.push_back(data);
}

void NParticles::Update(const mu_uint32 updateCount)
{
	auto &registry = Registry;
	for (mu_uint32 n = 0; n < updateCount; ++n)
	{
		using namespace TParticle;

		// Life Time check
		{
			const auto view = registry.view<
				Entity::LifeTime
			>();

			for (auto [entity, lifetime] : view.each())
			{
				if (--lifetime > 0) continue;
				registry.destroy(entity);
			}
		}

		auto view = registry.view<TParticle::Entity::Info>();
		//auto startTimer = std::chrono::high_resolution_clock::now();
		if constexpr (UseMultithread)
		{
			MUThreadsManager::Run(
				std::unique_ptr<NThreadExecutorBase>(
					new (std::nothrow) NThreadExecutorRangeIterator(
						view.begin(), view.end(),
						[&registry, &view](TParticle::EnttIterator begin, TParticle::EnttIterator end) {
							ParticleType type = ParticleType::Invalid;
							NMoveFunc func = nullptr;
							for (auto iter = begin; iter != end;)
							{
								const auto &entity = *iter;
								const auto &info = view.get<Entity::Info>(entity);
								if (info.Type != type)
								{
									type = info.Type;
									func = TParticles::GetMove(type);
								}
								if (func == nullptr) {
									++iter;
									continue;
								}

								iter = func(registry, view, iter, end);
							}
						}
					)
				)
			);

			MUThreadsManager::Run(
				std::unique_ptr<NThreadExecutorBase>(
					new (std::nothrow) NThreadExecutorRangeIterator(
						view.begin(), view.end(),
						[&registry, &view](TParticle::EnttIterator begin, TParticle::EnttIterator end) {
							ParticleType type = ParticleType::Invalid;
							NMoveFunc func = nullptr;
							for (auto iter = begin; iter != end;)
							{
								const auto entity = *iter;
								const auto &info = view.get<Entity::Info>(entity);
								if (info.Type != type)
								{
									type = info.Type;
									func = TParticles::GetAction(type);
								}
								if (func == nullptr) {
									++iter;
									continue;
								}

								iter = func(registry, view, iter, end);
							}
						}
					)
				)
			);
		}
		else
		{
			// Move
			{
				ParticleType type = ParticleType::Invalid;
				NMoveFunc func = nullptr;

				for (auto iter = view.begin(), last = view.end(); iter != last;)
				{
					const auto &entity = *iter;
					const auto &info = view.get<Entity::Info>(entity);
					if (info.Type != type)
					{
						type = info.Type;
						func = TParticles::GetMove(type);
					}
					if (func == nullptr) {
						++iter;
						continue;
					}

					iter = func(registry, view, iter, last);
				}
			}

			// Action
			{
				ParticleType type = ParticleType::Invalid;
				NActionFunc func = nullptr;

				for (auto iter = view.begin(), last = view.end(); iter != last;)
				{
					const auto &entity = *iter;
					const auto &info = view.get<Entity::Info>(entity);
					if (info.Type != type)
					{
						type = info.Type;
						func = TParticles::GetAction(type);
					}
					if (func == nullptr) {
						++iter;
						continue;
					}

					iter = func(registry, view, iter, last);
				}
			}
		}
		//auto endTimer = std::chrono::high_resolution_clock::now();
		//auto diff = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(endTimer - startTimer);
		//mu_info("[DEBUG] Particles Move : {}ms with {} elements", diff.count(), registry.size());
	}
}

void NParticles::Propagate()
{
	ParticleType type = ParticleType::Invalid;
	NCreateFunc create = nullptr;

	std::sort(
		PendingToCreate.begin(),
		PendingToCreate.end(),
		[](const NParticleData &lhs, const NParticleData &rhs) -> bool { return lhs.Type < rhs.Type; }
	);

	for (const auto &data : PendingToCreate)
	{
		if (data.Type != type)
		{
			type = data.Type;
			create = TParticles::GetCreate(type);
		}

		create(Registry, data);
	}

	if (PendingToCreate.size() > 0)
	{
		PendingToCreate.clear();

		using namespace TParticle;
		Registry.sort<Entity::Info>(
			[](const Entity::Info &lhs, const Entity::Info &rhs) -> bool {
				return Entity::GetSort(lhs) < Entity::GetSort(rhs);
			}
		);
	}
}

void NParticles::Render()
{
	return;
	using namespace TParticle;
	auto view = Registry.view<TParticle::Entity::Info>();

	// Render
	{
		ParticleType type = ParticleType::Invalid;
		NRenderFunc func = nullptr;

		for (auto iter = view.begin(), last = view.end(); iter != last;)
		{
			const auto &entity = *iter;
			const auto &info = view.get<Entity::Info>(entity);
			if (info.Type != type)
			{
				type = info.Type;
				func = TParticles::GetRender(type);
			}
			if (func == nullptr) {
				++iter;
				continue;
			}

			iter = func(Registry, view, iter, last, RenderBuffer);
		}
	}

	RenderBuffer.Count = 0;
}