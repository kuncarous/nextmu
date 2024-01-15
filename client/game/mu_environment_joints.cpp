#include "stdafx.h"
#include "mu_environment_joints.h"
#include "mu_resourcesmanager.h"
#include "mu_threadsmanager.h"
#include "mu_graphics.h"
#include "t_joint_base.h"
#include "t_joint_entity.h"
#include "mu_state.h"

using namespace TJoint;

const mu_boolean NJoints::Initialize()
{
	const auto device = MUGraphics::GetDevice();

	RenderBuffer.Program = MUResourcesManager::GetProgram("joint");
	if (RenderBuffer.Program == NInvalidShader)
	{
		return false;
	}

	const auto &swapchainDesc = MUGraphics::GetSwapChain()->GetDesc();
	RenderBuffer.FixedPipelineState.CombinedShader = RenderBuffer.Program;
	RenderBuffer.FixedPipelineState.RTVFormat = swapchainDesc.ColorBufferFormat;
	RenderBuffer.FixedPipelineState.DSVFormat = swapchainDesc.DepthBufferFormat;

	// Vertex Buffer
	{
		Diligent::BufferDesc bufferDesc;
		bufferDesc.Usage = Diligent::USAGE_DEFAULT;
		bufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
		bufferDesc.Size = sizeof(NJointVertex) * TJoint::MaxRenderCount * 4;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
		device->CreateBuffer(bufferDesc, nullptr, &buffer);
		if (buffer == nullptr)
		{
			return false;
		}

		RenderBuffer.VertexBuffer = buffer;
	}

	// Index Buffer
	{
		Diligent::BufferDesc bufferDesc;
		bufferDesc.Usage = Diligent::USAGE_DEFAULT;
		bufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
		bufferDesc.Size = sizeof(mu_uint32) * TJoint::MaxRenderCount * 6;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
		device->CreateBuffer(bufferDesc, nullptr, &buffer);
		if (buffer == nullptr)
		{
			return false;
		}

		RenderBuffer.IndexBuffer = buffer;
	}

	// Settings Uniform
	{
		Diligent::BufferDesc bufferDesc;
		bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
		bufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		bufferDesc.Size = sizeof(NJointSettings);

		Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
		device->CreateBuffer(bufferDesc, nullptr, &buffer);
		if (buffer == nullptr)
		{
			return false;
		}

		RenderBuffer.SettingsUniform = buffer;
	}

	const auto immediateContext = MUGraphics::GetImmediateContext();
	Diligent::StateTransitionDesc updateBarriers[2] = {
		Diligent::StateTransitionDesc(RenderBuffer.VertexBuffer, Diligent::RESOURCE_STATE_UNDEFINED, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE),
		Diligent::StateTransitionDesc(RenderBuffer.IndexBuffer, Diligent::RESOURCE_STATE_UNDEFINED, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE)
	};
	immediateContext->TransitionResourceStates(mu_countof(updateBarriers), updateBarriers);

	RenderBuffer.Groups.reserve(100);
	TJoint::Initialize();

	return true;
}

void NJoints::Destroy()
{
	RenderBuffer.Program = NInvalidShader;
	RenderBuffer.VertexBuffer.Release();
	RenderBuffer.IndexBuffer.Release();
	RenderBuffer.SettingsUniform.Release();
}

void NJoints::Create(const NJointData &data)
{
	PendingToCreate.push_back(data);
}

void NJoints::Update()
{
	const auto updateCount = MUState::GetUpdateCount();

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
							TJoint::Template *_template = nullptr;
							for (auto iter = begin; iter != end;)
							{
								const auto &entity = *iter;
								const auto &info = view.get<Entity::Info>(entity);
								if (info.Type != type)
								{
									type = info.Type;
									_template = TJoint::GetTemplate(type);
								}
								if (_template == nullptr) {
									++iter;
									continue;
								}

								iter = _template->Move(registry, view, iter, end);
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
							TJoint::Template *_template = nullptr;
							for (auto iter = begin; iter != end;)
							{
								const auto entity = *iter;
								const auto &info = view.get<Entity::Info>(entity);
								if (info.Type != type)
								{
									type = info.Type;
									_template = TJoint::GetTemplate(type);
								}
								if (_template == nullptr) {
									++iter;
									continue;
								}

								iter = _template->Action(registry, view, iter, end);
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
				TJoint::Template *_template = nullptr;

				for (auto iter = view.begin(), last = view.end(); iter != last;)
				{
					const auto &entity = *iter;
					const auto &info = view.get<Entity::Info>(entity);
					if (info.Type != type)
					{
						type = info.Type;
						_template = TJoint::GetTemplate(type);
					}
					if (_template == nullptr) {
						++iter;
						continue;
					}

					iter = _template->Move(registry, view, iter, last);
				}
			}

			// Action
			{
				JointType type = JointType::Invalid;
				TJoint::Template *_template = nullptr;

				for (auto iter = view.begin(), last = view.end(); iter != last;)
				{
					const auto &entity = *iter;
					const auto &info = view.get<Entity::Info>(entity);
					if (info.Type != type)
					{
						type = info.Type;
						_template = TJoint::GetTemplate(type);
					}
					if (_template == nullptr) {
						++iter;
						continue;
					}

					iter = _template->Action(registry, view, iter, last);
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
	TJoint::Template *_template = nullptr;

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
			_template = TJoint::GetTemplate(type);
		}
		if (_template == nullptr) continue;

		_template->Create(Registry, data);
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
						TJoint::Template *_template = nullptr;
						for (auto iter = begin; iter != end;)
						{
							const auto entity = *iter;
							const auto &info = view.get<Entity::Info>(entity);
							if (info.Type != type)
							{
								type = info.Type;
								_template = TJoint::GetTemplate(type);
							}
							if (_template == nullptr) {
								++iter;
								continue;
							}

							iter = _template->Render(registry, view, iter, end, renderBuffer);
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
		TJoint::Template *_template = nullptr;

		for (auto iter = view.begin(), last = view.end(); iter != last;)
		{
			const auto &entity = *iter;
			const auto &info = view.get<Entity::Info>(entity);
			if (info.Type != type)
			{
				type = info.Type;
				_template = TJoint::GetTemplate(type);
			}
			if (func == nullptr) {
				++iter;
				continue;
			}

			iter = _template->Render(Registry, view, iter, last, RenderBuffer);
		}
	}
#endif

	const auto &renderTargetDesc = MUGraphics::GetRenderTargetDesc();
	auto &fixedState = RenderBuffer.FixedPipelineState;
	fixedState.RTVFormat = renderTargetDesc.ColorFormat;
	fixedState.DSVFormat = renderTargetDesc.DepthStencilFormat;

	// Render Groups
	{
		JointType type = JointType::Invalid;
		TJoint::Template *_template = nullptr;
		for (const auto renderGroup : RenderBuffer.Groups)
		{
			if (renderGroup.Count == 0) continue;
			if (renderGroup.Type != type)
			{
				type = renderGroup.Type;
				_template = TJoint::GetTemplate(type);
			}
			if (_template == nullptr) {
				continue;
			}
			_template->RenderGroup(renderGroup, RenderBuffer);
		}
	}

	if (RenderBuffer.RequireTransition == true)
	{
		const auto immediateContext = MUGraphics::GetImmediateContext();
		Diligent::StateTransitionDesc updateBarriers[2] = {
			Diligent::StateTransitionDesc(RenderBuffer.VertexBuffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE),
			Diligent::StateTransitionDesc(RenderBuffer.IndexBuffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE)
		};
		immediateContext->TransitionResourceStates(mu_countof(updateBarriers), updateBarriers);
		RenderBuffer.RequireTransition = false;
	}

	//auto endTimer = std::chrono::high_resolution_clock::now();
	//auto diff = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(endTimer - startTimer);
	//mu_info("[DEBUG] Joints Render : {}ms with {} elements", diff.count(), Registry.size());

	RenderBuffer.Groups.clear();
}