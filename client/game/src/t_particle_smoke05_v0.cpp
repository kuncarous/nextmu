#include "mu_precompiled.h"
#include "t_particle_smoke05_v0.h"
#include "t_particle_macros.h"
#include "mu_resourcesmanager.h"
#include "mu_graphics.h"
#include "mu_renderstate.h"
#include "mu_state.h"

using namespace TParticle;
constexpr auto Type = ParticleType::Smoke05_V0;
constexpr auto LifeTime = 32;
constexpr auto LightDivisor = 1.0f / 32.0f;
static const mu_char* ParticleID = "smoke05_v0";
static const mu_char *TextureID = "smoke05";

const NDynamicPipelineState DynamicPipelineState = {
	.CullMode = Diligent::CULL_MODE_FRONT,
	.DepthWrite = false,
	.DepthFunc = Diligent::COMPARISON_FUNC_LESS_EQUAL,
	.SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA,
	.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
	.SrcBlendAlpha = Diligent::BLEND_FACTOR_SRC_ALPHA,
	.DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
};
constexpr mu_float IsPremultipliedAlpha = static_cast<mu_float>(false);
constexpr mu_float IsLinear = static_cast<mu_float>(false);

static TParticleSmoke05V0 Instance;
static NGraphicsTexture* texture = nullptr;

TParticleSmoke05V0::TParticleSmoke05V0()
{
	TParticle::Template::TemplateTypes.insert(std::make_pair(ParticleID, Type));
	TParticle::Template::Templates.insert(std::make_pair(Type, this));
}

void TParticleSmoke05V0::Initialize()
{
	texture = MUResourcesManager::GetResourcesManager()->GetTexture(TextureID);
}

void TParticleSmoke05V0::Create(entt::registry &registry, const NParticleData &data)
{
	const auto* terrain = MURenderState::GetTerrain();
	const auto textureHeight = texture->GetHeight();

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

	const mu_float scale = glm::linearRand(0.32f, 0.64f) * data.Scale;
	glm::vec3 position = glm::vec3(
		data.Position.x + glm::linearRand(-8.0f, 8.0f),
		data.Position.y + glm::linearRand(-8.0f, 8.0f),
		data.Position.z
	);
	position.z = terrain->RequestHeight(position.x, position.y) + textureHeight * scale * 0.5f;
	registry.emplace<Entity::Position>(
		entity,
		Entity::Position{
			.StartPosition = data.Position,
			.Position = position,
			.Angle = glm::vec3(glm::linearRand(0.0f, 359.99f), data.Angle.y, data.Angle.z),
			.Velocity = RotateByAngle(
				glm::vec3(0.0f, 3.0f, 0.0f),
				data.Angle
			),
			.Scale = scale,
		}
	);

	const mu_float luminosity = static_cast<mu_float>(LifeTime) * LightDivisor;
	registry.emplace<Entity::Light>(
		entity,
		glm::vec4(luminosity, luminosity, luminosity, 1.0f)
	);

	registry.emplace<Entity::RenderGroup>(entity, NInvalidUInt32);
	registry.emplace<Entity::RenderIndex>(entity, 0);
	registry.emplace<Entity::RenderCount>(entity, 1);
}

EnttIterator TParticleSmoke05V0::Move(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last)
{
	using namespace TParticle;

	const auto *terrain = MURenderState::GetTerrain();
	const auto textureHeight = texture->GetHeight();

	for (; iter != last; ++iter)
	{
		const auto entity = *iter;
		auto &info = view.get<Entity::Info>(entity);
		if (info.Type != Type) break;

		auto [lifetime, position, light] = registry.get<Entity::LifeTime, Entity::Position, Entity::Light>(entity);

		position.Position += RotateByAngle(position.Velocity, position.Angle);
		position.Position += position.Velocity;
		position.Velocity *= 0.9f;
		position.Scale += 0.08f;
		position.Position.z = terrain->RequestHeight(position.Position.x, position.Position.y) + textureHeight * position.Scale * 0.5f;

		const mu_float luminosity = static_cast<mu_float>(lifetime) * LightDivisor;
		light = glm::vec4(luminosity, luminosity, luminosity, luminosity);
	}

	return iter;
}

EnttIterator TParticleSmoke05V0::Action(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last)
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

EnttIterator TParticleSmoke05V0::Render(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last, NRenderBuffer &renderBuffer)
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

void TParticleSmoke05V0::RenderGroup(const NRenderGroup &renderGroup, NRenderBuffer &renderBuffer)
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
			.Classify = NRenderClassify::PostAlpha,
			.View = 0,
			.Index = 1,
		}
	);
}