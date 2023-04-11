#include "stdafx.h"
#include "mu_environment_joints.h"
#include "mu_resourcesmanager.h"
#include "mu_threadsmanager.h"
#include "t_joints.h"
#include "t_joint_entity.h"

using namespace TJoint;
constexpr mu_boolean UseMultithread = false;

const mu_boolean NJoints::Initialize()
{
#if NEXTMU_COMPRESSED_JOINTS == 1
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
	RenderBuffer.VertexBuffer = bgfx::createDynamicVertexBuffer(TJoint::MaxRenderCount * 4, RenderBuffer.Layout);
	if (bgfx::isValid(RenderBuffer.VertexBuffer) == false)
	{
		return false;
	}

	RenderBuffer.IndexBuffer = bgfx::createDynamicIndexBuffer(TJoint::MaxRenderCount * 6, BGFX_BUFFER_INDEX32);
	if (bgfx::isValid(RenderBuffer.IndexBuffer) == false)
	{
		return false;
	}

	RenderBuffer.TextureSampler = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
	if (bgfx::isValid(RenderBuffer.TextureSampler) == false)
	{
		return false;
	}

	RenderBuffer.Program = MUResourcesManager::GetProgram("joint");
	if (bgfx::isValid(RenderBuffer.Program) == false)
	{
		return false;
	}

	ThreadsRange.resize(MUThreadsManager::GetThreadsCount());

	return true;
}

void NJoints::Destroy()
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

	if (bgfx::isValid(RenderBuffer.TextureSampler))
	{
		bgfx::destroy(RenderBuffer.TextureSampler);
		RenderBuffer.TextureSampler = BGFX_INVALID_HANDLE;
	}
}

void NJoints::Create(const NJointData &data)
{
	PendingToCreate.push_back(data);
}

void NJoints::Update(const mu_uint32 updateCount)
{
	for (mu_uint32 n = 0; n < updateCount; ++n)
	{
		using namespace TJoint;

		// Life Time check
		{
			const auto view = Registry.view<
				Entity::LifeTime
			>();

			for (auto [entity, lifetime] : view.each())
			{
				if (--lifetime > 0) continue;
				Registry.destroy(entity);
			}
		}

		const auto view = Registry.view<
			Entity::Info
		>();
		const mu_uint32 entitiesCount = static_cast<mu_uint32>(view.size());

		// Move
		if constexpr (UseMultithread)
		{
			if (entitiesCount > 0)
			{
				auto &registry = Registry;
				auto &threadsRange = ThreadsRange;
				TThreading::SplitLoopIndex(entitiesCount, threadsRange);
				MUThreadsManager::Run(
					[&registry, &view, &threadsRange](const mu_uint32 threadIndex) -> void {
						const auto &range = threadsRange[threadIndex];
						JointType type = JointType::Invalid;
						NMoveFunc func = nullptr;
						for (auto iter = view.begin() + range.start, last = view.begin() + range.end; iter != last;)
						{
							const auto &entity = *iter;
							const auto &info = view.get<Entity::Info>(entity);
							if (info.Type != type)
							{
								type = info.Type;
								func = TJoints::GetMove(type);
							}
							if (func == nullptr) {
								++iter;
								continue;
							}

							iter = func(registry, iter, last);
						}
					}
				);

				MUThreadsManager::Run(
					[&registry, &view, &threadsRange](const mu_uint32 threadIndex) -> void {
						const auto &range = threadsRange[threadIndex];
						JointType type = JointType::Invalid;
						NActionFunc func = nullptr;
						for (auto iter = view.begin() + range.start, last = view.begin() + range.end; iter != last;)
						{
							const auto &entity = *iter;
							const auto &info = view.get<Entity::Info>(entity);
							if (info.Type != type)
							{
								type = info.Type;
								func = TJoints::GetAction(type);
							}
							if (func == nullptr) {
								++iter;
								continue;
							}

							iter = func(registry, iter, last);
						}
					}
				);
			}
		}
		else
		{
			// Move
			{
				JointType type = JointType::Invalid;
				NMoveFunc func = nullptr;

				for (auto iter = view.begin(), last = view.end(); iter != last;)
				{
					const auto &entity = *iter;
					const auto &info = Registry.get<Entity::Info>(entity);
					if (info.Type != type)
					{
						type = info.Type;
						func = TJoints::GetMove(type);
					}
					if (func == nullptr) {
						++iter;
						continue;
					}

					iter = func(Registry, iter, last);
				}
			}

			// Action
			{
				JointType type = JointType::Invalid;
				NActionFunc func = nullptr;

				for (auto iter = view.begin(), last = view.end(); iter != last;)
				{
					const auto &entity = *iter;
					const auto &info = Registry.get<Entity::Info>(entity);
					if (info.Type != type)
					{
						type = info.Type;
						func = TJoints::GetAction(type);
					}
					if (func == nullptr) {
						++iter;
						continue;
					}

					iter = func(Registry, iter, last);
				}
			}
		}
	}
}

void NJoints::Propagate()
{
	JointType type = JointType::Invalid;
	NCreateFunc create = nullptr;

	std::sort(
		PendingToCreate.begin(),
		PendingToCreate.end(),
		[](const NJointData &lhs, const NJointData &rhs) -> bool { return lhs.Type < rhs.Type; }
	);

	for (const auto &data : PendingToCreate)
	{
		if (data.Type != type)
		{
			type = data.Type;
			create = TJoints::GetCreate(type);
		}

		create(Registry, data);
	}

	if (PendingToCreate.size() > 0)
	{
		PendingToCreate.clear();

		using namespace TJoint;
		Registry.sort<Entity::Info>(
			[](const Entity::Info &lhs, const Entity::Info &rhs) -> bool {
				return Entity::GetSort(lhs) < Entity::GetSort(rhs);
			}
		);
	}
}

void NJoints::Render()
{
	using namespace TJoint;
	const auto view = Registry.view<
		Entity::Info
	>();

	// Render
	{
		JointType type = JointType::Invalid;
		NRenderFunc func = nullptr;

		for (auto iter = view.begin(), last = view.end(); iter != last;)
		{
			const auto &entity = *iter;
			const auto &info = Registry.get<Entity::Info>(entity);
			if (info.Type != type)
			{
				type = info.Type;
				func = TJoints::GetRender(type);
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