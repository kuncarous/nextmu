#include "stdafx.h"
#include "t_particle_smoke_v0.h"
#include "t_particle_macros.h"
#include "mu_resourcesmanager.h"
#include "mu_renderstate.h"
#include "mu_state.h"

namespace SmokeV0
{
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
	}

	void Create(entt::registry &registry, const NCreateData &data)
	{
		using namespace TParticle;
		const auto entity = registry.create();

		registry.emplace<Entity::Info>(
			entity,
			Entity::Info{
				.Layer = data.Layer,
				.LifeTime = LifeTime,
				.Type = Type,
			}
		);

		registry.emplace<Entity::Position>(
			entity,
			Entity::Position{
				.StartPosition = data.Position,
				.Position = data.Position,
				.Angle = glm::vec3(
					glm::linearRand(0.0f, 360.0f),
					data.Angle.y,
					data.Angle.z
				),
				.Velocity = glm::vec3(
					glm::linearRand(-2.0f, 2.0f),
					0.0f,
					glm::linearRand(1.0f, 3.0f)
				),
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
	}

	EnttIterator Move(entt::registry &registry, EnttIterator iter, EnttIterator last)
	{
		using namespace TParticle;

		for (; iter != last; ++iter)
		{
			const auto entity = *iter;
			auto &info = registry.get<Entity::Info>(entity);
			if (info.Type != Type) break;
			if (--info.LifeTime == 0) {
				registry.destroy(entity);
				continue;
			}

			auto &gravity = registry.get<Entity::Gravity>(entity);
			gravity += 0.2f;

			auto &position = registry.get<Entity::Position>(entity);
			position.Position = MovePosition(position.Position, position.Angle, position.Velocity);
			position.Position.z += gravity;
			position.Scale += 0.05f;

			const mu_float luminosity = static_cast<mu_float>(info.LifeTime) * LightDivisor;
			auto &light = registry.get<Entity::Light>(entity);
			light = glm::vec4(luminosity, luminosity, luminosity, 1.0f);
		}

		return iter;
	}

	EnttIterator Action(entt::registry &registry, EnttIterator iter, EnttIterator last)
	{
		using namespace TParticle;

		for (; iter != last; ++iter)
		{
			const auto entity = *iter;
			const auto &info = registry.get<Entity::Info>(entity);
			if (info.Type != Type) break;

			// ACTION HERE
		}

		return iter;
	}

	EnttIterator Render(entt::registry &registry, EnttIterator iter, EnttIterator last, NRenderBuffer &renderBuffer)
	{
		using namespace TParticle;

		static const NTexture *texture = nullptr;
		if (texture == nullptr)
		{
			texture = MUResourcesManager::GetTexture(TextureID);
		}

		const mu_float textureWidth = static_cast<mu_float>(texture->GetWidth());
		const mu_float textureHeight = static_cast<mu_float>(texture->GetHeight());

		cglm::mat4 projection, view;
		MURenderState::GetProjection(projection);
		MURenderState::GetView(view);

		const auto startIndex = renderBuffer.Count;
		mu_uint32 vertex = 0;
		for (; iter != last; ++iter)
		{
			const auto entity = *iter;
			const auto &info = registry.get<Entity::Info>(entity);
			if (info.Type != Type) break;

			const auto &position = registry.get<Entity::Position>(entity);
			const auto &light = registry.get<Entity::Light>(entity);
			const auto &rotation = registry.get<Entity::Rotation>(entity);

			const mu_float width = textureWidth * position.Scale * 0.5f;
			const mu_float height = textureHeight * position.Scale * 0.5f;

			RenderBillboardSpriteWithRotation(renderBuffer, vertex, view, position.Position, rotation, width, height, light);

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
