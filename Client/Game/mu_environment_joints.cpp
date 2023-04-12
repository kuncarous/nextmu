#include "stdafx.h"
#include "mu_environment_joints.h"
#include "mu_resourcesmanager.h"
#include "mu_threadsmanager.h"
#include "t_joints.h"
#include "t_joint_entity.h"

using namespace TJoint;

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
	RenderBuffer.Groups.reserve(100);

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
	auto &registry = Registry;
	for (mu_uint32 n = 0; n < updateCount; ++n)
	{
		using namespace TJoint;

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

		auto view = registry.view<Entity::Info>();
		//auto startTimer = std::chrono::high_resolution_clock::now();

#if ENABLE_JOINT_UPDATE_MULTITHREAD == 1
		{
			MUThreadsManager::Run(
				std::unique_ptr<NThreadExecutorBase>(
					new (std::nothrow) NThreadExecutorRangeIterator(
						view.begin(), view.end(),
						[&registry, &view](TJoint::EnttIterator begin, TJoint::EnttIterator end) {
							JointType type = JointType::Invalid;
							NMoveFunc func = nullptr;
							for (auto iter = begin; iter != end;)
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
						[&registry, &view](TJoint::EnttIterator begin, TJoint::EnttIterator end) {
							JointType type = JointType::Invalid;
							NMoveFunc func = nullptr;
							for (auto iter = begin; iter != end;)
							{
								const auto entity = *iter;
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

								iter = func(registry, view, iter, end);
							}
						}
					)
				)
			);
		}
#else
		{
			// Move
			{
				JointType type = JointType::Invalid;
				NMoveFunc func = nullptr;

				for (auto iter = view.begin(), last = view.end(); iter != last;)
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

					iter = func(registry, view, iter, last);
				}
			}

			// Action
			{
				JointType type = JointType::Invalid;
				NActionFunc func = nullptr;

				for (auto iter = view.begin(), last = view.end(); iter != last;)
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

					iter = func(registry, view, iter, last);
				}
			}
		}
#endif

		//auto endTimer = std::chrono::high_resolution_clock::now();
		//auto diff = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(endTimer - startTimer);
		//mu_info("[DEBUG] Joints Move : {}ms with {} elements", diff.count(), registry.size());
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

	//auto startTimer = std::chrono::high_resolution_clock::now();
	auto &groups = RenderBuffer.Groups;

	// Calculate
	{
		JointType type = JointType::Invalid;
		mu_int32 group = -1;

		mu_uint32 index = 0;
		auto view = Registry.view<Entity::Info, Entity::RenderGroup, Entity::RenderIndex, Entity::RenderCount>();
		for (auto iter = view.begin(); iter != view.end(); ++iter)
		{
			const auto entity = *iter;
			auto [
				info,
				renderGroup,
				renderIndex,
				renderCount
			] = view.get<
				Entity::Info,
				Entity::RenderGroup,
				Entity::RenderIndex,
				Entity::RenderCount
			>(entity);

			if (index + renderCount > MaxRenderCount) {
				renderGroup = NInvalidUInt32;
				continue;
			}

			if (type != info.Type)
			{
				groups.push_back(
					NRenderGroup{
						.Type = info.Type,
						.Index = index,
						.Count = 0,
					}
				);
				++group;
				type = info.Type;
			}

			auto &rgroup = groups[group];
			rgroup.Count += renderCount;

			renderGroup = group;
			renderIndex = index;
			index += renderCount;
		}
	}

#if ENABLE_JOINT_RENDER_MULTITHREAD == 1
	{
		auto &registry = Registry;
		auto view = registry.view<Entity::Info>();
		auto &renderBuffer = RenderBuffer;

		MUThreadsManager::Run(
			std::unique_ptr<NThreadExecutorBase>(
				new (std::nothrow) NThreadExecutorRangeIterator(
					view.begin(), view.end(),
					[&registry, &view, &renderBuffer](TJoint::EnttIterator begin, TJoint::EnttIterator end) {
						JointType type = JointType::Invalid;
						NRenderFunc func = nullptr;
						for (auto iter = begin; iter != end;)
						{
							const auto entity = *iter;
							const auto &info = view.get<Entity::Info>(entity);
							if (info.Type != type)
							{
								type = info.Type;
								func = TJoints::GetRender(type);
							}
							if (func == nullptr) {
								++iter;
								continue;
							}

							iter = func(registry, view, iter, end, renderBuffer);
						}
					}
				)
			)
		);
	}
#else
	{
		auto view = Registry.view<Entity::Info>();
		JointType type = JointType::Invalid;
		NRenderFunc func = nullptr;

		for (auto iter = view.begin(), last = view.end(); iter != last;)
		{
			const auto &entity = *iter;
			const auto &info = view.get<Entity::Info>(entity);
			if (info.Type != type)
			{
				type = info.Type;
				func = TJoints::GetRender(type);
			}
			if (func == nullptr) {
				++iter;
				continue;
			}

			iter = func(Registry, view, iter, last, RenderBuffer);
		}
	}
#endif

	// Render Groups
	{
		JointType type = JointType::Invalid;
		NRenderGroupFunc func = nullptr;
		for (const auto renderGroup : RenderBuffer.Groups)
		{
			if (renderGroup.Count == 0) continue;
			if (renderGroup.Type != type)
			{
				type = renderGroup.Type;
				func = TJoints::GetRenderGroup(type);
			}
			if (func == nullptr) {
				continue;
			}
			func(renderGroup, RenderBuffer);
			}
		}

	//auto endTimer = std::chrono::high_resolution_clock::now();
	//auto diff = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(endTimer - startTimer);
	//mu_info("[DEBUG] Joints Render : {}ms with {} elements", diff.count(), Registry.size());

	RenderBuffer.Groups.clear();
}