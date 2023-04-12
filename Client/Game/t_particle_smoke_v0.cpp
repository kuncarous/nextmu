#include "stdafx.h"
#include "t_particle_smoke_v0.h"
#include "t_particle_macros.h"
#include "mu_resourcesmanager.h"
#include "mu_renderstate.h"
#include "mu_state.h"

namespace SmokeV0
{
	using namespace TParticle;
	constexpr auto Type = ParticleType::Smoke_V0;
	constexpr auto LifeTime = 16;
	constexpr auto LightDivisor = 1.0f / 8.0f;
	const mu_char *TextureID = "smoke_v0";

	constexpr mu_uint64 RenderState = (
		BGFX_STATE_WRITE_RGB |
		BGFX_STATE_WRITE_A |
		BGFX_STATE_CULL_CW |
		BGFX_STATE_DEPTH_TEST_LEQUAL |
		BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE) |
		BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD)
		);

	void Register(NInvokes &invokes)
	{
		REGISTER_INVOKE(Create);
		REGISTER_INVOKE(Move);
		REGISTER_INVOKE(Action);
		REGISTER_INVOKE(Render);
		REGISTER_INVOKE(RenderGroup);
	}

	void Create(entt::registry &registry, const NParticleData &data)
	{
		using namespace TParticle;
		const auto entity = registry.create();

		registry.emplace<Entity::Info>(
			entity,
			Entity::Info {
				.Layer = data.Layer,
				.Type = Type,
			}
		);

		registry.emplace<Entity::LifeTime>(entity, LifeTime);

		registry.emplace<Entity::Position>(
			entity,
			Entity::Position {
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

	EnttIterator Move(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last)
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

	EnttIterator Action(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last)
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

	static const NTexture *texture = nullptr;
	EnttIterator Render(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last, NRenderBuffer &renderBuffer)
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

	void RenderGroup(const NRenderGroup &renderGroup, const NRenderBuffer &renderBuffer)
	{
		if (texture == nullptr) texture = MUResourcesManager::GetTexture(TextureID);
		if (texture == nullptr) return;

		cglm::mat4 projection;
		MURenderState::GetProjection(projection);
		bgfx::update(renderBuffer.VertexBuffer, renderGroup.Index * 4, bgfx::makeRef(renderBuffer.Vertices.data() + renderGroup.Index * 4, sizeof(NRenderVertex) * renderGroup.Count * 4));
		bgfx::update(renderBuffer.IndexBuffer, renderGroup.Index * 6, bgfx::makeRef(renderBuffer.Indices.data() + renderGroup.Index * 6, sizeof(mu_uint32) * renderGroup.Count * 6));
		bgfx::setState(RenderState);
		bgfx::setUniform(renderBuffer.Projection, projection);
		bgfx::setTexture(0, renderBuffer.TextureSampler, texture->GetTexture());
		bgfx::setVertexBuffer(0, renderBuffer.VertexBuffer, renderGroup.Index * 4, renderGroup.Count * 4);
		bgfx::setIndexBuffer(renderBuffer.IndexBuffer, renderGroup.Index * 6, renderGroup.Count * 6);
		bgfx::submit(0, renderBuffer.Program);
	}
}
