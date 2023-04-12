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

	EnttIterator Render(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last, TParticle::NRenderBuffer &renderBuffer)
	{
		using namespace TParticle;

		static const NTexture *texture = nullptr;
		if (texture == nullptr)
		{
			texture = MUResourcesManager::GetTexture(TextureID);
		}

		const mu_float textureWidth = static_cast<mu_float>(texture->GetWidth());
		const mu_float textureHeight = static_cast<mu_float>(texture->GetHeight());

		cglm::mat4 projection, gview;
		MURenderState::GetProjection(projection);
		MURenderState::GetView(gview);

		const auto startIndex = renderBuffer.Count;
		mu_uint32 vertex = 0;
		for (; iter != last; ++iter)
		{
			const auto entity = *iter;
			const auto &info = view.get<Entity::Info>(entity);
			if (info.Type != Type) break;

			const auto [position, light] = registry.get<Entity::Position, Entity::Light>(entity);

			const mu_float width = textureWidth * position.Scale * 0.5f;
			const mu_float height = textureHeight * position.Scale * 0.5f;

			RenderBillboardSprite(renderBuffer, vertex, gview, position.Position, width, height, light);

			if (renderBuffer.Count >= MaxRenderCount) {
				iter = last;
				break;
			}
		}

		const auto renderCount = renderBuffer.Count - startIndex;
		if (renderCount > 0)
		{
			bgfx::update(renderBuffer.VertexBuffer, startIndex * 4, bgfx::copy(renderBuffer.Vertices.data() + startIndex * 4, sizeof(NRenderVertex) * renderCount * 4));
			bgfx::update(renderBuffer.IndexBuffer, startIndex * 6, bgfx::copy(renderBuffer.Indices.data() + startIndex * 6, sizeof(mu_uint32) * renderCount * 6));
			bgfx::setState(RenderState);
			bgfx::setUniform(renderBuffer.Projection, projection);
			bgfx::setTexture(0, renderBuffer.TextureSampler, texture->GetTexture());
			bgfx::setVertexBuffer(0, renderBuffer.VertexBuffer, startIndex * 4, renderCount * 4);
			bgfx::setIndexBuffer(renderBuffer.IndexBuffer, startIndex * 6, renderCount * 6);
			bgfx::submit(0, renderBuffer.Program);
		}

		return iter;
	}
}
