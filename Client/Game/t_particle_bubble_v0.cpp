#include "stdafx.h"
#include "t_particle_bubble_v0.h"
#include "t_particle_macros.h"
#include "mu_resourcesmanager.h"
#include "mu_graphics.h"
#include "mu_renderstate.h"
#include "mu_state.h"

using namespace TParticle;
constexpr auto Type = ParticleType::Bubble_V0;
constexpr auto LightDivisor = 1.0f / 8.0f;
static const mu_char *TextureID = "bubble_v0";
constexpr mu_float FrameDivisor = 3;
constexpr mu_float UVMultiplier = 0.25f;
constexpr mu_float UVOffset = 0.005f;
constexpr mu_float USize = 0.25f - 0.01f;
constexpr mu_float VSize = 0.25f - 0.01f;

const NDynamicPipelineState DynamicPipelineState = {
	.DepthWrite = false,
	.DepthFunc = Diligent::COMPARISON_FUNC_LESS_EQUAL,
	.SrcBlend = Diligent::BLEND_FACTOR_ONE,
	.DestBlend = Diligent::BLEND_FACTOR_ONE,
	.BlendOp = Diligent::BLEND_OPERATION_ADD,
};

static TParticleBubbleV0 Instance;

TParticleBubbleV0::TParticleBubbleV0()
{
	TParticle::Template::Templates.insert(std::make_pair(Type, this));
}

void TParticleBubbleV0::Create(entt::registry &registry, const NParticleData &data)
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

	registry.emplace<Entity::LifeTime>(entity, glm::linearRand(30, 40));

	registry.emplace<Entity::Position>(
		entity,
		Entity::Position{
			.StartPosition = data.Position,
			.Position = data.Position,
			.Scale = glm::linearRand(0.12f, 0.3f),
		}
	);

	registry.emplace<Entity::Light>(
		entity,
		glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
	);

	registry.emplace<Entity::Frame>(
		entity,
		0
	);

	registry.emplace<Entity::RenderGroup>(entity, NInvalidUInt32);
	registry.emplace<Entity::RenderIndex>(entity, 0);
	registry.emplace<Entity::RenderCount>(entity, 1);
}

EnttIterator TParticleBubbleV0::Move(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last)
{
	using namespace TParticle;

	for (; iter != last; ++iter)
	{
		const auto entity = *iter;
		auto &info = view.get<Entity::Info>(entity);
		if (info.Type != Type) break;

		auto [position, frame] = registry.get<Entity::Position, Entity::Frame>(entity);
		position.Position += glm::linearRand(glm::vec3(-25.0f, -25.0f, 25.0f), glm::vec3(25.0f, 25.0f, 75.0f)) * position.Scale;
		frame = (frame + 1) % 9;
	}

	return iter;
}

EnttIterator TParticleBubbleV0::Action(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last)
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
EnttIterator TParticleBubbleV0::Render(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last, NRenderBuffer &renderBuffer)
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

		const auto [position, light, frame, renderGroup, renderIndex] = registry.get<Entity::Position, Entity::Light, Entity::Frame, Entity::RenderGroup, Entity::RenderIndex>(entity);
		if (renderGroup.t == NInvalidUInt32) continue;

		const auto uoffset = static_cast<mu_float>(frame % 3) * UVMultiplier + UVOffset;
		const auto voffset = static_cast<mu_float>(frame / 3) * UVMultiplier + UVOffset;

		const mu_float width = textureWidth * position.Scale * 0.5f;
		const mu_float height = textureHeight * position.Scale * 0.5f;

		RenderBillboardSprite(renderBuffer, renderGroup, renderIndex, gview, position.Position, width, height, light, glm::vec4(uoffset, voffset, uoffset + USize, voffset + VSize));
	}

	return iter;
}

void TParticleBubbleV0::RenderGroup(const NRenderGroup &renderGroup, NRenderBuffer &renderBuffer)
{
	if (texture == nullptr) texture = MUResourcesManager::GetTexture(TextureID);
	if (texture == nullptr) return;

	auto renderManager = MUGraphics::GetRenderManager();
	auto immediateContext = MURenderState::GetImmediateContext();

	immediateContext->UpdateBuffer(
		renderBuffer.VertexBuffer,
		sizeof(NParticleVertex) * renderGroup.Index * 4,
		sizeof(NParticleVertex) * renderGroup.Count * 4,
		renderBuffer.Vertices.data() + renderGroup.Index * 4,
		Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
	);
	immediateContext->UpdateBuffer(
		renderBuffer.IndexBuffer,
		sizeof(mu_uint32) * renderGroup.Index * 6,
		sizeof(mu_uint32) * renderGroup.Count * 6,
		renderBuffer.Indices.data() + renderGroup.Index * 6,
		Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
	);

	auto pipelineState = GetPipelineState(renderBuffer.FixedPipelineState, DynamicPipelineState);
	if (pipelineState->StaticInitialized == false)
	{
		pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "ModelViewProj")->Set(MURenderState::GetProjUniform());
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
			.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
			.Flags = Diligent::SET_VERTEX_BUFFERS_FLAG_RESET,
		}
	);
	renderManager->SetIndexBuffer(
		RSetIndexBuffer{
			.IndexBuffer = renderBuffer.IndexBuffer,
			.ByteOffset = 0,
			.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
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
