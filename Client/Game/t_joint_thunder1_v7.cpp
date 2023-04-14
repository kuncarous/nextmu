#include "stdafx.h"
#include "t_joint_thunder1_v7.h"
#include "t_joint_tails.h"
#include "mu_resourcesmanager.h"
#include "mu_renderstate.h"
#include "mu_state.h"

constexpr auto Type = JointType::Thunder1_V7;
constexpr auto LifeTime = 20;
constexpr auto LightDivisor = 1.0f / 1.2f;
static const mu_char *TextureID = "thunder_v0";

// TO DO : Remove this and check why they decided to generate a beam with 5 tails
constexpr mu_int32 tailOffset = 5;

constexpr mu_uint64 RenderState = (
	BGFX_STATE_WRITE_RGB |
	BGFX_STATE_WRITE_A |
	//BGFX_STATE_CULL_CW |
	BGFX_STATE_DEPTH_TEST_LEQUAL |
	BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE) |
	BGFX_STATE_BLEND_EQUATION(BGFX_STATE_BLEND_EQUATION_ADD)
);

static TJointThunder1V7 Instance;

TJointThunder1V7::TJointThunder1V7()
{
	TJoint::Template::Templates.insert(std::make_pair(Type, this));
}

void TJointThunder1V7::Create(TJoint::EnttRegistry &registry, const NJointData &data)
{
	using namespace TJoint;
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
			.Position = data.Position + glm::vec3(
				glm::linearRand(-5.0f, 5.0f),
				glm::linearRand(-5.0f, 5.0f),
				1050.0f
			),
			.TargetPosition = data.TargetPosition,
			.Scale = data.Scale,
		}
	);

	registry.emplace<Entity::Light>(
		entity,
		glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
	);

	registry.emplace<Entity::Tails>(
		entity,
		Entity::Tails{
			.Begin = 0,
			.Count = 0,
			.MaxCount = MaxTails,
		}
	);

	registry.emplace<Entity::RenderGroup>(entity, NInvalidUInt32);
	registry.emplace<Entity::RenderIndex>(entity, 0);
	registry.emplace<Entity::RenderCount>(entity, 0);
}

TJoint::EnttIterator TJointThunder1V7::Move(TJoint::EnttRegistry &registry, TJoint::EnttView &view, TJoint::EnttIterator iter, TJoint::EnttIterator last)
{
	using namespace TJoint;
	for (; iter != last; ++iter)
	{
		const auto entity = *iter;
		auto &info = view.get<Entity::Info>(entity);
		if (info.Type != Type) break;

		auto [
			lifetime,
				position,
				light,
				tails,
				renderCount
		] = registry.get<
			Entity::LifeTime,
				Entity::Position,
				Entity::Light,
				Entity::Tails,
				Entity::RenderCount
		>(entity);

		const auto rotation = glm::quat(glm::radians(position.Angle));
		position.Position = position.StartPosition;

		for (mu_int16 n = 0, maxTails = tails.MaxCount - 5; n < maxTails; ++n)
		{
			position.Position += glm::vec3(
				glm::linearRand(-10.0f, 10.0f),
				glm::linearRand(-10.0f, 10.0f),
				-20.0f
			);
			CreateTail(tails, position.Position, rotation, position.Scale);
		}

		const glm::vec3 direction = (position.TargetPosition - position.Position) * 0.2f;
		for (mu_int16 n = tails.MaxCount - 5, maxTails = tails.MaxCount - 1; n < maxTails; ++n)
		{
			position.Position += direction;
			position.Position += glm::vec3(
				glm::linearRand(-10.0f, 10.0f),
				glm::linearRand(-10.0f, 10.0f),
				0.0f
			);
			CreateTail(tails, position.Position, rotation, position.Scale);
		}

		position.Position = position.TargetPosition;
		CreateTail(tails, position.Position, rotation, position.Scale);

		if (lifetime < 4) light *= LightDivisor;

		renderCount = (tails.Count - tailOffset) * 2;
	}

	return iter;
}

TJoint::EnttIterator TJointThunder1V7::Action(TJoint::EnttRegistry &registry, TJoint::EnttView &view, TJoint::EnttIterator iter, TJoint::EnttIterator last)
{
	using namespace TJoint;
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
TJoint::EnttIterator TJointThunder1V7::Render(TJoint::EnttRegistry &registry, TJoint::EnttView &view, TJoint::EnttIterator iter, TJoint::EnttIterator last, TJoint::NRenderBuffer &renderBuffer)
{
	using namespace TJoint;
	const mu_float scroll = glm::mod(MUState::GetWorldTime(), 1000.0f) * 0.002f;
	for (; iter != last; ++iter)
	{
		const auto entity = *iter;
		const auto &info = view.get<Entity::Info>(entity);
		if (info.Type != Type) break;

		const auto [position, light, tails, renderGroup, renderIndex] = registry.get<Entity::Position, Entity::Light, Entity::Tails, Entity::RenderGroup, Entity::RenderIndex>(entity);
		if (renderGroup.t == NInvalidUInt32) continue;

		mu_uint32 rindex = renderIndex;
		const auto maxTails = static_cast<mu_float>(tails.MaxCount - 1);
		for (mu_int32 j1 = tails.Begin + tailOffset, j2 = (j1 + 1) % tails.MaxCount, n = tailOffset, count = tails.Count; n < count; j1 = j2, j2 = (j2 + 1) % tails.MaxCount, ++n)
		{
			const auto &tail1 = tails.Tails[j1];
			const auto &tail2 = tails.Tails[j2];

			constexpr mu_float V1 = 0.0f;
			constexpr mu_float V2 = 1.0f;
			mu_float L1 = static_cast<mu_float>(tails.Count - n) / maxTails;
			mu_float L2 = static_cast<mu_float>(tails.Count - (n + 1)) / maxTails;

			const glm::vec3 tp1[4] = {
				tail1[2],
				tail1[3],
				tail2[3],
				tail2[2]
			};
			RenderTail(
				renderBuffer,
				renderGroup,
				rindex++,
				tp1,
				light,
				glm::vec4(
					L1,
					V2,
					L2,
					V1
				)
			);

			L1 *= scroll;
			L2 *= scroll;

			const glm::vec3 tp2[4] = {
				tail1[0],
				tail1[1],
				tail2[1],
				tail2[0]
			};
			RenderTail(
				renderBuffer,
				renderGroup,
				rindex++,
				tp2,
				light,
				glm::vec4(
					L1,
					V1,
					L2,
					V2
				)
			);
		}
	}

	return iter;
}

void TJointThunder1V7::RenderGroup(const TJoint::NRenderGroup &renderGroup, const TJoint::NRenderBuffer &renderBuffer)
{
	if (texture == nullptr) texture = MUResourcesManager::GetTexture(TextureID);
	if (texture == nullptr) return;

	bgfx::update(renderBuffer.VertexBuffer, renderGroup.Index * 4, bgfx::makeRef(renderBuffer.Vertices.data() + renderGroup.Index * 4, sizeof(TJoint::NRenderVertex) * renderGroup.Count * 4));
	bgfx::update(renderBuffer.IndexBuffer, renderGroup.Index * 6, bgfx::makeRef(renderBuffer.Indices.data() + renderGroup.Index * 6, sizeof(mu_uint32) * renderGroup.Count * 6));
	bgfx::setState(RenderState);
	bgfx::setTexture(0, renderBuffer.TextureSampler, texture->GetTexture());
	bgfx::setVertexBuffer(0, renderBuffer.VertexBuffer, renderGroup.Index * 4, renderGroup.Count * 4);
	bgfx::setIndexBuffer(renderBuffer.IndexBuffer, renderGroup.Index * 6, renderGroup.Count * 6);
	bgfx::submit(0, renderBuffer.Program);
}