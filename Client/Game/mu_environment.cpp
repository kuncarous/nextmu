#include "stdafx.h"
#include "mu_environment.h"
#include "mu_state.h"
#include "mu_modelrenderer.h"

void NEnvironment::Reset()
{
	const auto updateCount = MUState::GetUpdateCount();

	if (updateCount > 0)
	{
		Terrain->Reset();
	}
}

void NEnvironment::Update()
{
	const auto updateCount = MUState::GetUpdateCount();

	if (updateCount > 0)
	{
		Terrain->Update();
	}

	const auto objectsView = Objects.view<
		NEntity::Renderable,
		NEntity::Attachment,
		NEntity::Light,
		NEntity::RenderState,
		NEntity::Skeleton,
		NEntity::Position,
		NEntity::Animation
	>();

	for (auto [entity, attachment, light, renderState, skeleton, position, animation] : objectsView.each())
	{
		CalculateLight(position, light, renderState);

		skeleton.Instance.PlayAnimation(attachment.Model, animation.CurrentAction, animation.PriorAction, animation.CurrentFrame, animation.PriorFrame, attachment.Model->GetPlaySpeed() * MUState::GetUpdateTime());
		skeleton.Instance.SetParent(
			position.Angle,
			position.Position,
			position.Scale
		);
		skeleton.Instance.Animate(
			attachment.Model,
			{
				.Action = animation.CurrentAction,
				.Frame = animation.CurrentFrame,
			},
			{
				.Action = animation.PriorAction,
				.Frame = animation.PriorFrame,
			},
			glm::vec3(0.0f, 0.0f, 0.0f)
		);

		skeleton.SkeletonOffset = skeleton.Instance.Upload();
	}
}

void NEnvironment::Render()
{
	Terrain->ConfigureUniforms();
	Terrain->Render();

	const auto objectsView = Objects.view<NEntity::Renderable, NEntity::Attachment, NEntity::Light, NEntity::RenderState, NEntity::Skeleton>();
	for (auto [entity, attachment, light, renderState, skeleton] : objectsView.each())
	{
		if (skeleton.SkeletonOffset == NInvalidUInt32) continue;
		const NModelRenderConfig config = {
			.BoneOffset = skeleton.SkeletonOffset,
			.BodyOrigin = glm::vec3(0.0f, 0.0f, 0.0f),
			.BodyScale = 1.0f,
			.EnableLight = renderState.LightEnable,
			.BodyLight = renderState.BodyLight,
		};
		MUModelRenderer::RenderBody(attachment.Model, config);
	}
}

void NEnvironment::CalculateLight(
	const NEntity::Position &position,
	const NEntity::Light &light,
	NEntity::RenderState &renderState
)
{
	switch (light.Mode)
	{
	case EntityLightMode::Terrain:
		{
			const auto &terrain = light.Settings.Terrain;
			const glm::vec3 terrainLight = (
				terrain.PrimaryLight
				? Terrain->CalculatePrimaryLight(position.Position[0], position.Position[1])
				: Terrain->CalculateBackLight(position.Position[0], position.Position[1])
			);
			renderState.BodyLight = glm::vec4(terrain.Color + terrainLight, 1.0f);
		}
		break;

	case EntityLightMode::Fixed:
		{
			const auto &fixed = light.Settings.Fixed;
			renderState.BodyLight = glm::vec4(fixed.Color, 1.0f);
		}
		break;

	case EntityLightMode::SinWorldTime:
		{
			const auto &worldTime = light.Settings.WorldTime;
			mu_float luminosity = glm::sin(MUState::GetWorldTime() * worldTime.TimeMultiplier) * worldTime.Multiplier + worldTime.Add;
			renderState.BodyLight = glm::vec4(luminosity, luminosity, luminosity, 1.0f);
		}
		break;
	}
}