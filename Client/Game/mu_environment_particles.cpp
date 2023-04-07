#include "stdafx.h"
#include "mu_environment_particles.h"
#include "mu_resourcesmanager.h"
#include "t_particles.h"
#include "t_particle_entity.h"

const mu_boolean NParticles::Initialize()
{
	RenderBuffer.Layout
		.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Int16, true, true)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true, true)
		.end();
	RenderBuffer.VertexBuffer = bgfx::createDynamicVertexBuffer(MaxRenderCount * 4, RenderBuffer.Layout);
	if (bgfx::isValid(RenderBuffer.VertexBuffer) == false)
	{
		return false;
	}

	RenderBuffer.IndexBuffer = bgfx::createDynamicIndexBuffer(MaxRenderCount * 6, BGFX_BUFFER_INDEX32);
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

void NParticles::Create(
	const mu_uint8 layer,
	const ParticleType type,
	const glm::vec3 position,
	const glm::vec3 angle,
	const glm::vec3 light,
	const mu_float scale
)
{
	NCreateData data = {
		.Layer = layer,
		.Type = type,
		.Position = position,
		.Angle = angle,
		.Light = light,
		.Scale = scale,
	};
	PendingToCreate.push_back(data);
}

void NParticles::Update(const mu_uint32 updateCount)
{
	using namespace TParticle;
	const auto view = Registry.view<
		Entity::Info
	>();

	// Move
	if (updateCount > 0)
	{
		ParticleType type = ParticleType::Invalid;
		NMoveFunc func = nullptr;

		for (auto iter = view.begin(), last = view.end(); iter != last;)
		{
			const auto &entity = *iter;
			const auto &info = Registry.get<Entity::Info>(entity);
			if (info.Type != type)
			{
				type = info.Type;
				func = TParticles::GetMove(type);
			}
			if (func == nullptr) {
				++iter;
				continue;
			}

			iter = func(Registry, iter, last);
		}
	}

	// Action
	if (updateCount > 0)
	{
		ParticleType type = ParticleType::Invalid;
		NActionFunc func = nullptr;

		for (auto iter = view.begin(), last = view.end(); iter != last;)
		{
			const auto &entity = *iter;
			const auto &info = Registry.get<Entity::Info>(entity);
			if (info.Type != type)
			{
				type = info.Type;
				func = TParticles::GetAction(type);
			}
			if (func == nullptr) {
				++iter;
				continue;
			}

			iter = func(Registry, iter, last);
		}
	}
}

void NParticles::Propagate()
{
	ParticleType type = ParticleType::Invalid;
	NCreateFunc create = nullptr;

	std::sort(
		PendingToCreate.begin(),
		PendingToCreate.end(),
		[](const NCreateData &lhs, const NCreateData &rhs) -> bool { return lhs.Type < rhs.Type; }
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
	using namespace TParticle;
	const auto view = Registry.view<
		Entity::Info
	>();

	// Render
	{
		ParticleType type = ParticleType::Invalid;
		NRenderFunc func = nullptr;

		for (auto iter = view.begin(), last = view.end(); iter != last;)
		{
			const auto &entity = *iter;
			const auto &info = Registry.get<Entity::Info>(entity);
			if (info.Type != type)
			{
				type = info.Type;
				func = TParticles::GetRender(type);
			}
			if (func == nullptr) {
				++iter;
				continue;
			}

			iter = func(Registry, iter, last, RenderBuffer);
		}
	}

	RenderBuffer.Count = 0;
}