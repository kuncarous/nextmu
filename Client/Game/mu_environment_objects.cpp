#include "stdafx.h"
#include "mu_environment_objects.h"
#include "mu_environment.h"
#include "mu_entity.h"
#include "mu_camera.h"
#include "mu_modelrenderer.h"
#include "mu_bboxrenderer.h"
#include "mu_state.h"
#include "mu_renderstate.h"
#include "mu_threadsmanager.h"

const mu_boolean NObjects::Initialize()
{
	return true;
}

void NObjects::Destroy()
{

}

void NObjects::Update()
{
	const auto updateTime = MUState::GetUpdateTime();
	const auto frustum = MURenderState::GetCamera()->GetFrustum();
	const auto environment = MURenderState::GetEnvironment();

	const auto view = Registry.view<
		NEntity::NRenderable,
		NEntity::NAttachment,
		NEntity::NLight,
		NEntity::NRenderState,
		NEntity::NSkeleton,
		NEntity::NPosition,
		NEntity::NAnimation,
		NEntity::NBoundingBox
	>();

	MUThreadsManager::Run(
		std::unique_ptr<NThreadExecutorBase>(
			new (std::nothrow) NThreadExecutorIterator(
				view.begin(), view.end(),
				[&view, environment, frustum, updateTime](const entt::entity entity) -> void {
					auto [attachment, light, renderState, skeleton, position, animation, boundingBox] = view.get<
						NEntity::NAttachment,
						NEntity::NLight,
						NEntity::NRenderState,
						NEntity::NSkeleton,
						NEntity::NPosition,
						NEntity::NAnimation,
						NEntity::NBoundingBox
					>(entity);

					skeleton.Instance.SetParent(
						position.Angle,
						position.Position,
						position.Scale
					);
					skeleton.Instance.PlayAnimation(attachment.Base, animation.CurrentAction, animation.PriorAction, animation.CurrentFrame, animation.PriorFrame, attachment.Base->GetPlaySpeed() * updateTime);

					NCompressedMatrix viewModel;
					viewModel.Set(
						position.Angle,
						position.Position,
						position.Scale
					);

					const auto model = attachment.Base;
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

					if (!renderState.Flags.Visible) return;

					environment->CalculateLight(position, light, renderState);
					skeleton.Instance.Animate(
						attachment.Base,
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
			)
		)
	);
}

void NObjects::Render()
{
	const auto view = Registry.view<NEntity::NRenderable, NEntity::NAttachment, NEntity::NRenderState, NEntity::NSkeleton>();
	for (auto [entity, attachment, renderState, skeleton] : view.each())
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
		MUModelRenderer::RenderBody(skeleton.Instance, attachment.Base, config);
	}

#if RENDER_BBOX
	const auto bboxView = Objects.view<NEntity::NRenderable, NEntity::NRenderState, NEntity::NBoundingBox>();
	for (auto [entity, renderState, boundingBox] : bboxView.each())
	{
		if (!renderState.Flags.Visible) continue;
		MUBBoxRenderer::Render(boundingBox);
	}
#endif
}

void NObjects::Clear()
{
	Registry.clear();
}

const entt::entity NObjects::Add(
	const TObject::Settings object
)
{
	auto &registry = Registry;
	const auto entity = registry.create();
	registry.emplace<NEntity::NAttachment>(
		entity,
		NEntity::NAttachment{
			.Base = object.Model,
		}
	);

	if (object.Renderable)
	{
		registry.emplace<NEntity::NRenderable>(entity);
	}

	if (object.Interactive)
	{
		registry.emplace<NEntity::NInteractive>(entity);
	}

	NEntity::NLightSettings lightSettings;
	switch (object.Light.Mode)
	{
	case EntityLightMode::Terrain:
		{
			lightSettings.Terrain.Color = object.Light.Color;
			lightSettings.Terrain.Intensity = object.Light.LightIntensity;
			lightSettings.Terrain.PrimaryLight = object.Light.PrimaryLight;
		}
		break;

	case EntityLightMode::Fixed:
		{
			lightSettings.Fixed.Color = object.Light.Color;
		}
		break;

	case EntityLightMode::SinWorldTime:
		{
			lightSettings.WorldTime.TimeMultiplier = object.Light.TimeMultiplier;
			lightSettings.WorldTime.Multiplier = object.Light.LightMultiplier;
			lightSettings.WorldTime.Add = object.Light.LightAdd;
		}
		break;
	}

	registry.emplace<NEntity::NLight>(
		entity,
		object.Light.Mode,
		lightSettings
	);

	registry.emplace<NEntity::NRenderState>(
		entity,
		NEntity::NRenderFlags{
			.Visible = false,
			.LightEnable = object.LightEnable,
		},
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);

	NSkeletonInstance instance;
	instance.SetParent(
		object.Angle,
		glm::vec3(0.0f, 0.0f, 0.0f),
		1.0f
	);

	registry.emplace<NEntity::NSkeleton>(
		entity,
		NEntity::NSkeleton{
			.SkeletonOffset = NInvalidUInt32,
			.Instance = instance,
		}
	);

	registry.emplace<NEntity::NBoundingBox>(
		entity,
		object.BBoxMin,
		object.BBoxMax
	);

	registry.emplace<NEntity::NPosition>(
		entity,
		NEntity::NPosition{
			.Position = object.Position,
			.Angle = object.Angle,
			.Scale = object.Scale,
		}
	);

	registry.emplace<NEntity::NAnimation>(
		entity,
		NEntity::NAnimation{}
	);

	return entity;
}

void NObjects::Remove(const entt::entity entity)
{
	auto &registry = Registry;
	registry.destroy(entity);
}