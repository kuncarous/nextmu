#include "stdafx.h"
#include "t_joint_thunder01_v7.h"
#include "t_joint_tails.h"
#include "mu_resourcesmanager.h"
#include "mu_graphics.h"
#include "mu_renderstate.h"
#include "mu_state.h"
#include <MapHelper.hpp>

using namespace TJoint;
constexpr auto Type = JointType::Thunder01_V7;
constexpr auto LifeTime = 20;
constexpr auto LightDivisor = 1.0f / 1.2f;
static const mu_char *JointID = "thunder01_v7";
static const mu_char *TextureID = "thunder01";

// TO DO : Remove this and check why they decided to generate a beam with 5 tails
constexpr mu_int32 tailOffset = 5;

const NDynamicPipelineState DynamicPipelineState = {
	.CullMode = Diligent::CULL_MODE_NONE,
	.DepthWrite = false,
	.DepthFunc = Diligent::COMPARISON_FUNC_LESS_EQUAL,
	.SrcBlend = Diligent::BLEND_FACTOR_ONE,
	.DestBlend = Diligent::BLEND_FACTOR_ONE,
	.SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE,
	.DestBlendAlpha = Diligent::BLEND_FACTOR_ONE,
};
constexpr mu_boolean IsPremultipliedAlpha = true;

static TJointThunder01V7 Instance;
static NGraphicsTexture* texture = nullptr;

TJointThunder01V7::TJointThunder01V7()
{
	TJoint::Template::Templates.insert(std::make_pair(Type, this));
}

void TJointThunder01V7::Initialize()
{
	texture = MUResourcesManager::GetTexture(TextureID);
}

void TJointThunder01V7::Create(TJoint::EnttRegistry &registry, const NJointData &data)
{
	using namespace TJoint;
	const auto entity = registry.create();

	registry.emplace<Entity::Info>(
		entity,
		Entity::Info{
			.Layer = data.Layer,
			.Type = Type,
		}
	);

	registry.emplace<Entity::LifeTime>(entity, LifeTime);

	registry.emplace<Entity::Position>(
		entity,
		Entity::Position{
			.StartPosition = data.Position,
			.Position = data.Position + glm::vec3(
				glm::linearRand(-5.0f, 5.0f),
				glm::linearRand(-5.0f, 5.0f),
				1050.0f
			),
			.TargetPosition = data.TargetPosition,
			.Scale = data.Scale,
		}
	);

	registry.emplace<Entity::Light>(
		entity,
		glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
	);

	registry.emplace<Entity::Tails>(
		entity,
		Entity::Tails{
			.Begin = 0,
			.Count = 0,
			.MaxCount = MaxTails,
		}
	);

	registry.emplace<Entity::RenderGroup>(entity, NInvalidUInt32);
	registry.emplace<Entity::RenderIndex>(entity, 0);
	registry.emplace<Entity::RenderCount>(entity, 0);
}

TJoint::EnttIterator TJointThunder01V7::Move(TJoint::EnttRegistry &registry, TJoint::EnttView &view, TJoint::EnttIterator iter, TJoint::EnttIterator last)
{
	using namespace TJoint;
	for (; iter != last; ++iter)
	{
		const auto entity = *iter;
		auto &info = view.get<Entity::Info>(entity);
		if (info.Type != Type) break;

		auto [
			lifetime,
				position,
				light,
				tails,
				renderCount
		] = registry.get<
			Entity::LifeTime,
				Entity::Position,
				Entity::Light,
				Entity::Tails,
				Entity::RenderCount
		>(entity);

		const auto rotation = glm::quat(glm::radians(position.Angle));
		position.Position = position.StartPosition;

		for (mu_int16 n = 0, maxTails = tails.MaxCount - 5; n < maxTails; ++n)
		{
			position.Position += glm::vec3(
				glm::linearRand(-10.0f, 10.0f),
				glm::linearRand(-10.0f, 10.0f),
				-20.0f
			);
			CreateTail(tails, position.Position, rotation, position.Scale);
		}

		const glm::vec3 direction = (position.TargetPosition - position.Position) * 0.2f;
		for (mu_int16 n = tails.MaxCount - 5, maxTails = tails.MaxCount - 1; n < maxTails; ++n)
		{
			position.Position += direction;
			position.Position += glm::vec3(
				glm::linearRand(-10.0f, 10.0f),
				glm::linearRand(-10.0f, 10.0f),
				0.0f
			);
			CreateTail(tails, position.Position, rotation, position.Scale);
		}

		position.Position = position.TargetPosition;
		CreateTail(tails, position.Position, rotation, position.Scale);

		if (lifetime < 4) light *= LightDivisor;

		renderCount = (tails.Count - tailOffset) * 2;
	}

	return iter;
}

TJoint::EnttIterator TJointThunder01V7::Action(TJoint::EnttRegistry &registry, TJoint::EnttView &view, TJoint::EnttIterator iter, TJoint::EnttIterator last)
{
	using namespace TJoint;
	for (; iter != last; ++iter)
	{
		const auto entity = *iter;
		const auto &info = view.get<Entity::Info>(entity);
		if (info.Type != Type) break;

		// ACTION HERE
	}

	return iter;
}

TJoint::EnttIterator TJointThunder01V7::Render(TJoint::EnttRegistry &registry, TJoint::EnttView &view, TJoint::EnttIterator iter, TJoint::EnttIterator last, TJoint::NRenderBuffer &renderBuffer)
{
	using namespace TJoint;
	const mu_float scroll = glm::mod(MUState::GetWorldTime(), 1000.0f) * 0.002f;
	for (; iter != last; ++iter)
	{
		const auto entity = *iter;
		const auto &info = view.get<Entity::Info>(entity);
		if (info.Type != Type) break;

		const auto [light, tails, renderGroup, renderIndex] = registry.get<Entity::Light, Entity::Tails, Entity::RenderGroup, Entity::RenderIndex>(entity);
		if (renderGroup.t == NInvalidUInt32) continue;

		mu_uint32 rindex = renderIndex;
		const auto maxTails = static_cast<mu_float>(tails.MaxCount - 1);
		for (mu_int32 j1 = tails.Begin + tailOffset, j2 = (j1 + 1) % tails.MaxCount, n = tailOffset, count = tails.Count; n < count; j1 = j2, j2 = (j2 + 1) % tails.MaxCount, ++n)
		{
			const auto &tail1 = tails.Tails[j1];
			const auto &tail2 = tails.Tails[j2];

			constexpr mu_float V1 = 0.0f;
			constexpr mu_float V2 = 1.0f;
			mu_float L1 = static_cast<mu_float>(tails.Count - n) / maxTails;
			mu_float L2 = static_cast<mu_float>(tails.Count - (n + 1)) / maxTails;

			const glm::vec3 tp1[4] = {
				tail1[2],
				tail1[3],
				tail2[3],
				tail2[2]
			};
			RenderTail(
				renderBuffer,
				renderGroup,
				rindex++,
				tp1,
				light,
				glm::vec4(
					L1,
					V2,
					L2,
					V1
				)
			);

			L1 *= scroll;
			L2 *= scroll;

			const glm::vec3 tp2[4] = {
				tail1[0],
				tail1[1],
				tail2[1],
				tail2[0]
			};
			RenderTail(
				renderBuffer,
				renderGroup,
				rindex++,
				tp2,
				light,
				glm::vec4(
					L1,
					V1,
					L2,
					V2
				)
			);
		}
	}

	return iter;
}

void TJointThunder01V7::RenderGroup(const TJoint::NRenderGroup &renderGroup, TJoint::NRenderBuffer &renderBuffer)
{
	auto renderManager = MUGraphics::GetRenderManager();
	auto immediateContext = MUGraphics::GetImmediateContext();

	if (renderBuffer.RequireTransition == false)
	{
		Diligent::StateTransitionDesc updateBarriers[2] = {
			Diligent::StateTransitionDesc(renderBuffer.VertexBuffer, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE),
			Diligent::StateTransitionDesc(renderBuffer.IndexBuffer, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE)
		};
		immediateContext->TransitionResourceStates(mu_countof(updateBarriers), updateBarriers);
		renderBuffer.RequireTransition = true;
	}

	immediateContext->UpdateBuffer(
		renderBuffer.VertexBuffer,
		sizeof(NJointVertex) * renderGroup.Index * 4,
		sizeof(NJointVertex) * renderGroup.Count * 4,
		renderBuffer.Vertices.data() + renderGroup.Index * 4,
		Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE
	);
	immediateContext->UpdateBuffer(
		renderBuffer.IndexBuffer,
		sizeof(mu_uint32) * renderGroup.Index * 6,
		sizeof(mu_uint32) * renderGroup.Count * 6,
		renderBuffer.Indices.data() + renderGroup.Index * 6,
		Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE
	);

	// Update Model Settings
	{
		auto uniform = renderBuffer.SettingsBuffer.Allocate();
		uniform->IsPremultipliedAlpha = IsPremultipliedAlpha;
		renderManager->UpdateBufferWithMap(
			RUpdateBufferWithMap{
				.ShouldReleaseMemory = false,
				.Buffer = renderBuffer.SettingsUniform,
				.Data = uniform,
				.Size = sizeof(NJointSettings),
				.MapType = Diligent::MAP_WRITE,
				.MapFlags = Diligent::MAP_FLAG_DISCARD,
			}
		);
	}

	auto pipelineState = GetPipelineState(renderBuffer.FixedPipelineState, DynamicPipelineState);
	if (pipelineState->StaticInitialized == false)
	{
		pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "cbCameraAttribs")->Set(MURenderState::GetCameraUniform());
		pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "JointSettings")->Set(renderBuffer.SettingsUniform);
		pipelineState->StaticInitialized = true;
	}

	NResourceId resourceIds[1] = { texture->GetId() };
	auto binding = ShaderResourcesBindingManager.GetShaderBinding(pipelineState->Id, pipelineState->Pipeline, mu_countof(resourceIds), resourceIds);
	if (binding->Initialized == false)
	{
		binding->Binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(texture->GetTexture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
		binding->Initialized = true;
	}

	renderManager->SetPipelineState(pipelineState);
	renderManager->SetVertexBuffer(
		RSetVertexBuffer{
			.StartSlot = 0,
			.Buffer = renderBuffer.VertexBuffer.RawPtr(),
			.Offset = 0,
			.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY,
			.Flags = Diligent::SET_VERTEX_BUFFERS_FLAG_NONE,
		}
	);
	renderManager->SetIndexBuffer(
		RSetIndexBuffer{
			.IndexBuffer = renderBuffer.IndexBuffer,
			.ByteOffset = 0,
			.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY,
		}
	);
	renderManager->CommitShaderResources(
		RCommitShaderResources{
			.ShaderResourceBinding = binding,
		}
	);

	renderManager->DrawIndexed(
		RDrawIndexed{
			.Attribs = Diligent::DrawIndexedAttribs(renderGroup.Count * 6, Diligent::VT_UINT32, Diligent::DRAW_FLAG_VERIFY_ALL, 1, renderGroup.Index * 6, renderGroup.Index * 4)
		},
		RCommandListInfo{
			.Type = NDrawOrderType::Classifier,
			.View = 0,
			.Index = 0,
		}
	);
}