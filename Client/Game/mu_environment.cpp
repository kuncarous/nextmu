#include "stdafx.h"
#include "mu_environment.h"
#include "mu_state.h"
#include "mu_camera.h"
#include "mu_modelrenderer.h"
#include "mu_bboxrenderer.h"
#include "mu_renderstate.h"

#define RENDER_BBOX (0)

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

	const auto frustum = MURenderState::GetCamera()->GetFrustum();

	const auto visibilityView = Objects.view<
		NEntity::Renderable,
		NEntity::Attachment,
		NEntity::RenderState,
		NEntity::BoundingBox,
		NEntity::Position
	>();
	for (auto [entity, attachment, renderState, boundingBox, position] : visibilityView.each())
	{
		NCompressedMatrix viewModel;
		viewModel.Set(
			position.Angle,
			position.Position,
			position.Scale
		);

		const auto model = attachment.Model;
		if (model->HasMeshes() && model->HasGlobalBBox())
		{
			const auto &bbox = model->GetGlobalBBox();
			boundingBox.Min = Transform(bbox.Min, viewModel);
			boundingBox.Max = Transform(bbox.Max, viewModel);
		}
		else
		{
			boundingBox.Min = Transform(boundingBox.Min, viewModel);
			boundingBox.Max = Transform(boundingBox.Max, viewModel);
		}
		boundingBox.Order();

		renderState.Flags.Visible = frustum->IsBoxVisible(boundingBox.Min, boundingBox.Max);
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
		if (!renderState.Flags.Visible) continue;

		CalculateLight(position, light, renderState);

		skeleton.Instance.SetParent(
			position.Angle,
			position.Position,
			position.Scale
		);
		skeleton.Instance.PlayAnimation(attachment.Model, animation.CurrentAction, animation.PriorAction, animation.CurrentFrame, animation.PriorFrame, attachment.Model->GetPlaySpeed() * MUState::GetUpdateTime());
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

	const auto objectsView = Objects.view<NEntity::Renderable, NEntity::Attachment, NEntity::RenderState, NEntity::Skeleton>();
	for (auto [entity, attachment, renderState, skeleton] : objectsView.each())
	{
		if (!renderState.Flags.Visible) continue;
		if (skeleton.SkeletonOffset == NInvalidUInt32) continue;
		const NRenderConfig config = {
			.BoneOffset = skeleton.SkeletonOffset,
			.BodyOrigin = glm::vec3(0.0f, 0.0f, 0.0f),
			.BodyScale = 1.0f,
			.EnableLight = renderState.Flags.LightEnable,
			.BodyLight = renderState.BodyLight,
		};
		MUModelRenderer::RenderBody(skeleton.Instance, attachment.Model, config);
	}

#if RENDER_BBOX
	const auto bboxView = Objects.view<NEntity::Renderable, NEntity::RenderState, NEntity::BoundingBox>();
	for (auto [entity, renderState, boundingBox] : bboxView.each())
	{
		if (!renderState.Flags.Visible) continue;
		MUBBoxRenderer::Render(boundingBox);
	}
#endif
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