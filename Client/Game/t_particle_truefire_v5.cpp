#include "stdafx.h"
#include "t_particle_truefire_v5.h"
#include "t_particle_macros.h"
#include "mu_resourcesmanager.h"
#include "mu_renderstate.h"

namespace TrueFireV5
{
	using namespace TParticle;
	constexpr auto Type = ParticleType::TrueFire_V5;
	constexpr auto LifeTime = 20;
	constexpr auto LightDivisor = 1.0f / 25.0f;
	const mu_char *TextureID = "truefire_v5";

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
				.Angle = data.Angle,
				.Velocity = glm::vec3(
					glm::linearRand(-2.0f, 2.0f),
					0.0f,
					glm::linearRand(1.0f, 3.0f)
				),
				.Scale = data.Scale,
			}
		);

		registry.emplace<Entity::Light>(
			entity,
			glm::vec4(static_cast<mu_float>(LifeTime) * LightDivisor, data.Light.x, data.Light.x, 1.0f)
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

			auto [lifetime, position, light] = registry.get<Entity::LifeTime, Entity::Position, Entity::Light>(entity);
			position.Position = MovePosition(position.Position, position.Angle, position.Velocity);
			position.Position.z += 1.0f;
			position.Velocity.x *= 0.95f;
			position.Scale = glm::max(position.Scale - 0.02f, 0.0f);
			light = glm::vec4(lifetime * LightDivisor, light.x, light.x, 1.0f);
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

			const auto [position, light, renderGroup, renderIndex] = registry.get<Entity::Position, Entity::Light, Entity::RenderGroup, Entity::RenderIndex>(entity);
			if (renderGroup.t == NInvalidUInt32) continue;

			const mu_float width = textureWidth * position.Scale * 0.5f;
			const mu_float height = textureHeight * position.Scale * 0.5f;

			RenderBillboardSprite(renderBuffer, renderGroup, renderIndex, gview, position.Position, width, height, light);
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
