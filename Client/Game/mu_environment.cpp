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

	const auto objectsView = Objects.view<NEntity::Attachment, NEntity::Skeleton, NEntity::Animation, NEntity::Position>();
	for (auto [entity, attachment, skeleton, animation, position] : objectsView.each())
	{
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
			position.Angle
		);

		skeleton.SkeletonOffset = skeleton.Instance.Upload();
	}
}

void NEnvironment::Render()
{
	Terrain->ConfigureUniforms();
	Terrain->Render();

	const auto objectsView = Objects.view<NEntity::Attachment, NEntity::Skeleton, NEntity::Position>();
	for (auto [entity, attachment, skeleton, position] : objectsView.each())
	{
		if (skeleton.SkeletonOffset == NInvalidUInt32) continue;
		MUModelRenderer::RenderBody(attachment.Model, skeleton.SkeletonOffset, position.Position, position.Scale);
	}
}