#include "stdafx.h"
#include "t_particle_smoke_v0.h"
#include "t_particle_macros.h"
#include "mu_resourcesmanager.h"
#include "mu_graphics.h"
#include "mu_renderstate.h"
#include "mu_state.h"

using namespace TParticle;
constexpr auto Type = ParticleType::Smoke_V0;
constexpr auto LifeTime = 16;
constexpr auto LightDivisor = 1.0f / 8.0f;
static const mu_char *TextureID = "smoke_v0";

const NDynamicPipelineState DynamicPipelineState = {
	.DepthWrite = false,
	.DepthFunc = Diligent::COMPARISON_FUNC_LESS_EQUAL,
	.SrcBlend = Diligent::BLEND_FACTOR_ONE,
	.DestBlend = Diligent::BLEND_FACTOR_ONE,
	.BlendOp = Diligent::BLEND_OPERATION_ADD,
};
constexpr mu_boolean IsPremultipliedAlpha = true;

static TParticleSmokeV0 Instance;

TParticleSmokeV0::TParticleSmokeV0()
{
	TParticle::Template::Templates.insert(std::make_pair(Type, this));
}

void TParticleSmokeV0::Create(entt::registry &registry, const NParticleData &data)
{
	using namespace TParticle;
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
			.Position = data.Position,
			.Scale = glm::linearRand(4.8f, 8.0f),
		}
	);

	const mu_float luminosity = static_cast<mu_float>(LifeTime) * LightDivisor;
	registry.emplace<Entity::Light>(
		entity,
		glm::vec4(luminosity, luminosity, luminosity, 1.0f)
	);

	registry.emplace<Entity::Rotation>(
		entity,
		glm::mod(MUState::GetWorldTime(), 360.0f)
	);

	registry.emplace<Entity::Gravity>(
		entity,
		0.0f
	);

	registry.emplace<Entity::RenderGroup>(entity, NInvalidUInt32);
	registry.emplace<Entity::RenderIndex>(entity, 0);
	registry.emplace<Entity::RenderCount>(entity, 1);
}

EnttIterator TParticleSmokeV0::Move(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last)
{
	using namespace TParticle;

	for (; iter != last; ++iter)
	{
		const auto entity = *iter;
		auto &info = view.get<Entity::Info>(entity);
		if (info.Type != Type) break;

		auto [lifetime, position, light, gravity] = registry.get<Entity::LifeTime, Entity::Position, Entity::Light, Entity::Gravity>(entity);

		gravity += 0.2f;
		position.Position.z += gravity;
		position.Scale += 0.05f;

		const mu_float luminosity = static_cast<mu_float>(lifetime) * LightDivisor;
		light = glm::vec4(luminosity, luminosity, luminosity, 1.0f);
	}

	return iter;
}

EnttIterator TParticleSmokeV0::Action(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last)
{
	using namespace TParticle;

	for (; iter != last; ++iter)
	{
		const auto entity = *iter;
		const auto &info = view.get<Entity::Info>(entity);
		if (info.Type != Type) break;

		// ACTION HERE
	}

	return iter;
}

static NGraphicsTexture *texture = nullptr;
EnttIterator TParticleSmokeV0::Render(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last, NRenderBuffer &renderBuffer)
{
	using namespace TParticle;

	if (texture == nullptr) texture = MUResourcesManager::GetTexture(TextureID);

	const mu_float textureWidth = static_cast<mu_float>(texture->GetWidth());
	const mu_float textureHeight = static_cast<mu_float>(texture->GetHeight());

	cglm::mat4 gview;
	MURenderState::GetView(gview);

	for (; iter != last; ++iter)
	{
		const auto entity = *iter;
		const auto &info = view.get<Entity::Info>(entity);
		if (info.Type != Type) break;

		const auto [position, light, rotation, renderGroup, renderIndex] = registry.get<Entity::Position, Entity::Light, Entity::Rotation, Entity::RenderGroup, Entity::RenderIndex>(entity);
		if (renderGroup.t == NInvalidUInt32) continue;

		const mu_float width = textureWidth * position.Scale * 0.5f;
		const mu_float height = textureHeight * position.Scale * 0.5f;

		RenderBillboardSpriteWithRotation(renderBuffer, renderGroup, renderIndex, gview, position.Position, rotation, width, height, light);
	}

	return iter;
}

void TParticleSmokeV0::RenderGroup(const NRenderGroup &renderGroup, NRenderBuffer &renderBuffer)
{
	if (texture == nullptr) texture = MUResourcesManager::GetTexture(TextureID);
	if (texture == nullptr) return;

	auto renderManager = MUGraphics::GetRenderManager();
	auto immediateContext = MUGraphics::GetImmediateContext();

	immediateContext->UpdateBuffer(
		renderBuffer.VertexBuffer,
		sizeof(NParticleVertex) * renderGroup.Index * 4,
		sizeof(NParticleVertex) * renderGroup.Count * 4,
		renderBuffer.Vertices.data() + renderGroup.Index * 4,
		renderBuffer.RequireTransition == false ? Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION : Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY
	);
	immediateContext->UpdateBuffer(
		renderBuffer.IndexBuffer,
		sizeof(mu_uint32) * renderGroup.Index * 6,
		sizeof(mu_uint32) * renderGroup.Count * 6,
		renderBuffer.Indices.data() + renderGroup.Index * 6,
		renderBuffer.RequireTransition == false ? Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION : Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY
	);
	renderBuffer.RequireTransition = true;

	// Update Model Settings
	{
		auto uniform = renderBuffer.SettingsBuffer.Allocate();
		uniform->IsPremultipliedAlpha = IsPremultipliedAlpha;
		renderManager->UpdateBufferWithMap(
			RUpdateBufferWithMap{
				.ShouldReleaseMemory = false,
				.Buffer = renderBuffer.SettingsUniform,
				.Data = uniform,
				.Size = sizeof(NParticleSettings),
				.MapType = Diligent::MAP_WRITE,
				.MapFlags = Diligent::MAP_FLAG_DISCARD,
			}
		);
	}

	auto pipelineState = GetPipelineState(renderBuffer.FixedPipelineState, DynamicPipelineState);
	if (pipelineState->StaticInitialized == false)
	{
		pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "ModelViewProj")->Set(MURenderState::GetProjUniform());
		pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "ParticleSettings")->Set(MURenderState::GetProjUniform());
		pipelineState->StaticInitialized = true;
	}

	NResourceId resourceIds[1] = { texture->GetId() };
	auto binding = GetShaderBinding(pipelineState, mu_countof(resourceIds), resourceIds);
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