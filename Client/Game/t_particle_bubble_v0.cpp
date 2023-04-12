#include "stdafx.h"
#include "t_particle_bubble_v0.h"
#include "t_particle_macros.h"
#include "mu_resourcesmanager.h"
#include "mu_renderstate.h"
#include "mu_state.h"

namespace BubbleV0
{
	using namespace TParticle;
	constexpr auto Type = ParticleType::Bubble_V0;
	constexpr auto LightDivisor = 1.0f / 8.0f;
	const mu_char *TextureID = "bubble_v0";
	constexpr mu_float FrameDivisor = 3;
	constexpr mu_float UVMultiplier = 0.25f;
	constexpr mu_float UVOffset = 0.005f;
	constexpr mu_float USize = 0.25f - 0.01f;
	constexpr mu_float VSize = 0.25f - 0.01f;

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
			Entity::Info {
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
	}

	EnttIterator Move(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last)
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

			const auto [position, light, frame] = registry.get<Entity::Position, Entity::Light, Entity::Frame>(entity);
			const auto uoffset = static_cast<mu_float>(frame % 3) * UVMultiplier + UVOffset;
			const auto voffset = static_cast<mu_float>(frame / 3) * UVMultiplier + UVOffset;

			const mu_float width = textureWidth * position.Scale * 0.5f;
			const mu_float height = textureHeight * position.Scale * 0.5f;

			RenderBillboardSprite(renderBuffer, vertex, gview, position.Position, width, height, light, glm::vec4(uoffset, voffset, uoffset + USize, voffset + VSize));

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
