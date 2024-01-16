#include "mu_precompiled.h"
#include "t_particle_effect_v5.h"
#include "t_particle_macros.h"
#include "mu_resourcesmanager.h"
#include "mu_graphics.h"
#include "mu_renderstate.h"
#include "mu_state.h"

using namespace TParticle;
constexpr auto Type = ParticleType::Effect_V5;
constexpr auto LifeTime = 20;
static const mu_char* ParticleID = "effect_v5";
static const mu_char *TextureID = "impack03";

const NDynamicPipelineState DynamicPipelineState = {
	.CullMode = Diligent::CULL_MODE_FRONT,
	.DepthWrite = false,
	.DepthFunc = Diligent::COMPARISON_FUNC_LESS_EQUAL,
	.SrcBlend = Diligent::BLEND_FACTOR_ONE,
	.DestBlend = Diligent::BLEND_FACTOR_ONE,
	.SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE,
	.DestBlendAlpha = Diligent::BLEND_FACTOR_ONE,
};
constexpr mu_float IsPremultipliedAlpha = static_cast<mu_float>(false);
constexpr mu_float IsLinear = static_cast<mu_float>(false);

static TParticleEffectV5 Instance;
static NGraphicsTexture* texture = nullptr;

TParticleEffectV5::TParticleEffectV5()
{
	TParticle::Template::TemplateTypes.insert(std::make_pair(ParticleID, Type));
	TParticle::Template::Templates.insert(std::make_pair(Type, this));
}

void TParticleEffectV5::Initialize()
{
	texture = MUResourcesManager::GetTexture(TextureID);
}

void TParticleEffectV5::Create(entt::registry &registry, const NParticleData &data)
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

	registry.emplace<Entity::LifeTime>(entity, LifeTime + glm::linearRand(0, 2));

	glm::vec3 position = glm::vec3(
		data.Position.x + glm::linearRand(-25.0f, 25.0f),
		data.Position.y + glm::linearRand(-25.0f, 25.0f),
		data.Position.z + glm::linearRand(-100.0f, 100.0f) + 150.0f
	);
	registry.emplace<Entity::Position>(
		entity,
		Entity::Position{
			.StartPosition = data.Position,
			.Position = position,
			.Angle = data.Angle,
			.Scale = glm::linearRand(0.26f, 0.39f),
		}
	);

	registry.emplace<Entity::Gravity>(entity, glm::linearRand(1.3f, 1.95f));

	registry.emplace<Entity::Light>(
		entity,
		glm::vec4(data.Light, 1.0f)
	);

	registry.emplace<Entity::RenderGroup>(entity, NInvalidUInt32);
	registry.emplace<Entity::RenderIndex>(entity, 0);
	registry.emplace<Entity::RenderCount>(entity, 1);
}

EnttIterator TParticleEffectV5::Move(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last)
{
	using namespace TParticle;

	for (; iter != last; ++iter)
	{
		const auto entity = *iter;
		auto &info = view.get<Entity::Info>(entity);
		if (info.Type != Type) break;

		auto [lifetime, position, gravity, light] = registry.get<Entity::LifeTime, Entity::Position, Entity::Gravity, Entity::Light>(entity);
		position.Position.z += gravity;
		light *= lifetime.t >= 10u ? 1.16f : (1.0f / 1.16f);
	}

	return iter;
}

EnttIterator TParticleEffectV5::Action(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last)
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

EnttIterator TParticleEffectV5::Render(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last, NRenderBuffer &renderBuffer)
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

		const auto [position, light, renderGroup, renderIndex] = registry.get<Entity::Position, Entity::Light, Entity::RenderGroup, Entity::RenderIndex>(entity);
		if (renderGroup.t == NInvalidUInt32) continue;

		const mu_float width = textureWidth * position.Scale * 0.5f;
		const mu_float height = textureHeight * position.Scale * 0.5f;

		RenderBillboardSprite(renderBuffer, renderGroup, renderIndex, gview, position.Position, width, height, light);
	}

	return iter;
}

void TParticleEffectV5::RenderGroup(const NRenderGroup &renderGroup, NRenderBuffer &renderBuffer)
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