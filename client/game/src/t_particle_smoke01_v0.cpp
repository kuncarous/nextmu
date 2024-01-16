#include "mu_precompiled.h"
#include "t_particle_smoke01_v0.h"
#include "t_particle_macros.h"
#include "mu_resourcesmanager.h"
#include "mu_graphics.h"
#include "mu_renderstate.h"
#include "mu_state.h"

using namespace TParticle;
constexpr auto Type = ParticleType::Smoke01_V0;
constexpr auto LifeTime = 16;
constexpr auto LightDivisor = 1.0f / 8.0f;
static const mu_char* ParticleID = "smoke01_v0";
static const mu_char *TextureID = "smoke01";

const NDynamicPipelineState DynamicPipelineState = {
	.CullMode = Diligent::CULL_MODE_FRONT,
	.DepthWrite = false,
	.DepthFunc = Diligent::COMPARISON_FUNC_LESS_EQUAL,
	.SrcBlend = Diligent::BLEND_FACTOR_ONE,
	.DestBlend = Diligent::BLEND_FACTOR_ONE,
	.SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE,
	.DestBlendAlpha = Diligent::BLEND_FACTOR_ONE,
};
constexpr mu_float IsPremultipliedAlpha = static_cast<mu_float>(true);
constexpr mu_float IsLinear = static_cast<mu_float>(false);

static TParticleSmoke01V0 Instance;
static NGraphicsTexture* texture = nullptr;

TParticleSmoke01V0::TParticleSmoke01V0()
{
	TParticle::Template::TemplateTypes.insert(std::make_pair(ParticleID, Type));
	TParticle::Template::Templates.insert(std::make_pair(Type, this));
}

void TParticleSmoke01V0::Initialize()
{
	texture = MUResourcesManager::GetTexture(TextureID);
}

void TParticleSmoke01V0::Create(entt::registry &registry, const NParticleData &data)
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

EnttIterator TParticleSmoke01V0::Move(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last)
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

EnttIterator TParticleSmoke01V0::Action(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last)
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

EnttIterator TParticleSmoke01V0::Render(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last, NRenderBuffer &renderBuffer)
{
	using namespace TParticle;

	const mu_float textureWidth = static_cast<mu_float>(texture->GetWidth());
	const mu_float textureHeight = static_cast<mu_float>(texture->GetHeight());

	glm::mat4 gview = MURenderState::GetView();

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

void TParticleSmoke01V0::RenderGroup(const NRenderGroup &renderGroup, NRenderBuffer &renderBuffer)
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
		sizeof(NParticleVertex) * renderGroup.Index * 4,
		sizeof(NParticleVertex) * renderGroup.Count * 4,
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
		uniform->IsLinear = IsLinear;
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
		pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "cbCameraAttribs")->Set(MURenderState::GetCameraUniform());
		pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "ParticleSettings")->Set(renderBuffer.SettingsUniform);
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