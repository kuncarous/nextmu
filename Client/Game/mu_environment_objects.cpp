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
#include "res_renders.h"

NObjects::NObjects(const NEnvironment *environment) : Environment(environment)
{}

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
	const auto environment = MURenderState::GetEnvironment();

	/*const auto view = Registry.view<
		NEntity::NRenderable,
		NEntity::NAttachment,
		NEntity::NLight,
		NEntity::NRenderState,
		NEntity::NSkeleton,
		NEntity::NPosition,
		NEntity::NAnimation,
		NEntity::NBoundingBoxes
	>();

	MUThreadsManager::Run(
		std::unique_ptr<NThreadExecutorBase>(
			new (std::nothrow) NThreadExecutorIterator(
				view.begin(), view.end(),
				[&view, environment, &renderSettings, updateTime](const entt::entity entity) -> void {
					auto [attachment, light, renderState, skeleton, position, animation, boundingBox] = view.get<
						NEntity::NAttachment,
						NEntity::NLight,
						NEntity::NRenderState,
						NEntity::NSkeleton,
						NEntity::NPosition,
						NEntity::NAnimation,
						NEntity::NBoundingBoxes
					>(entity);

					
				}
			)
		)
	);*/
}

void NObjects::PreRender(const NRenderSettings &renderSettings)
{
	const auto updateTime = MUState::GetUpdateTime();
	const auto environment = MURenderState::GetEnvironment();

	const auto view = Registry.view<
		NEntity::NRenderable,
		NEntity::NAttachment,
		NEntity::NLight,
		NEntity::NRenderState,
		NEntity::NSkeleton,
		NEntity::NPosition,
		NEntity::NAnimation,
		NEntity::NBoundingBoxes
	>();

	const auto distanceToCharacter = Environment->GetController()->GetDistanceToCharacter();
	const auto nearPoint = Environment->GetController()->GetNearPoint();
	MUThreadsManager::Run(
		std::unique_ptr<NThreadExecutorBase>(
			new (std::nothrow) NThreadExecutorIterator(
				view.begin(), view.end(),
				[&view, environment, &renderSettings, updateTime, distanceToCharacter, nearPoint](const entt::entity entity) -> void {
					auto [attachment, light, renderState, skeleton, position, animation, boundingBox] = view.get<
						NEntity::NAttachment,
						NEntity::NLight,
						NEntity::NRenderState,
						NEntity::NSkeleton,
						NEntity::NPosition,
						NEntity::NAnimation,
						NEntity::NBoundingBoxes
					>(entity);

					skeleton.Instance.SetParent(
						position.Angle,
						position.Position,
						position.Scale
					);

					NCompressedMatrix viewModel;
					viewModel.Set(
						position.Angle,
						position.Position,
						position.Scale
					);

					const auto model = attachment.Base;
					model->PlayAnimation(animation.CurrentAction, animation.PriorAction, animation.CurrentFrame, animation.PriorFrame, model->GetPlaySpeed(animation.CurrentAction) * updateTime);

					auto &bbox = boundingBox.Calculated;
					if (model->HasMeshes() && model->HasGlobalBBox())
					{
						const auto &globalBBox = model->GetGlobalBBox();
						bbox.Min = Transform(globalBBox.Min, viewModel);
						bbox.Max = Transform(globalBBox.Max, viewModel);
					}
					else
					{
						bbox.Min = Transform(boundingBox.Configured.Min, viewModel);
						bbox.Max = Transform(boundingBox.Configured.Max, viewModel);
					}
					bbox.Order();
					
					/* If we have parts to be processed then we animate the skeleton before checking if the object is visible */
					if (attachment.Parts.size() > 0)
					{
						skeleton.Instance.Animate(
							model,
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
					}

					for (auto &[type, part] : attachment.Parts)
					{
						const auto model = part.Model;
						const auto bone = part.IsLinked ? part.Link.Bone : 0;

						if (part.IsLinked == true)
						{
							auto &link = part.Link;
							auto &animation = link.Animation;
							model->PlayAnimation(animation.CurrentAction, animation.PriorAction, animation.CurrentFrame, animation.PriorFrame, model->GetPlaySpeed(animation.CurrentAction) * updateTime);
						}

						if (model->HasMeshes() && model->HasGlobalBBox())
						{
							const auto viewModel = skeleton.Instance.GetBone(bone);

							NBoundingBox tmpBBox;
							const auto &globalBBox = model->GetGlobalBBox();
							tmpBBox.Min = Transform(globalBBox.Min, viewModel);
							tmpBBox.Max = Transform(globalBBox.Max, viewModel);
							tmpBBox.Order();

							bbox.Min = glm::min(bbox.Min, tmpBBox.Min);
							bbox.Max = glm::max(bbox.Max, tmpBBox.Max);
							bbox.Order();
						}
					}

					const mu_boolean isVisible = Diligent::GetBoxVisibility(
						*renderSettings.Frustum,
						Diligent::BoundBox{
							.Min = Diligent::float3(bbox.Min.x, -bbox.Max.y, bbox.Min.z),
							.Max = Diligent::float3(bbox.Max.x, -bbox.Min.y, bbox.Max.z),
						}
					) != Diligent::BoxVisibility::Invisible;

					mu_boolean shadowVisible = false;
					if (renderSettings.ShadowFrustums != nullptr)
					{
						for (mu_uint32 n = 0; n < renderSettings.ShadowFrustumsNum; ++n)
						{
							renderState.ShadowVisible[n] = Diligent::GetBoxVisibility(
								renderSettings.ShadowFrustums[n],
								Diligent::BoundBox{
									.Min = Diligent::float3(bbox.Min.x, -bbox.Max.y, bbox.Min.z),
									.Max = Diligent::float3(bbox.Max.x, -bbox.Min.y, bbox.Max.z),
								},
								Diligent::FRUSTUM_PLANE_FLAG_OPEN_NEAR
								) != Diligent::BoxVisibility::Invisible;
							shadowVisible |= renderState.ShadowVisible[n];
						}
					}

					renderState.Flags.Visible = isVisible;

					if (!renderState.Flags.Visible && !shadowVisible) return;

					environment->CalculateLight(position, light, renderState);

					/* If we have parts to be processed then we animate the skeleton before checking if the object is visible */
					if (attachment.Parts.size() == 0)
					{
						skeleton.Instance.Animate(
							model,
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
					}
					skeleton.SkeletonOffset = skeleton.Instance.Upload();

					for (auto &[type, part] : attachment.Parts)
					{
						if (part.IsLinked == false) continue;
						const auto model = part.Model;
						auto &link = part.Link;
						auto &animation = link.Animation;
						auto &partSkeleton = link.Skeleton;
						const auto &renderAnimation = link.RenderAnimation;

						const auto boneMatrix = skeleton.Instance.GetBone(link.Bone);
						NCompressedMatrix transformMatrix{
							.Rotation = glm::quat(glm::radians(renderAnimation.Angle)),
							.Position = renderAnimation.Position,
							.Scale = renderAnimation.Scale
						};
						MixBones(boneMatrix, transformMatrix);

						partSkeleton.SetParent(transformMatrix);
						partSkeleton.Animate(
							model,
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
						link.SkeletonOffset = partSkeleton.Upload();
					}

					if (renderState.Flags.ShouldFade && distanceToCharacter > 0.0f)
					{
						auto &bboxMin = boundingBox.Calculated.Min;
						auto &bboxMax = boundingBox.Calculated.Max;
						Diligent::BoundBox bbox;
						bbox.Min = Diligent::float3(bboxMin.x, bboxMin.y, bboxMin.z);
						bbox.Max = Diligent::float3(bboxMax.x, bboxMax.y, bboxMax.z);
						const mu_float distance = Diligent::GetPointToBoxDistance(bbox, nearPoint);
						if (distance <= distanceToCharacter)
						{
							if (renderState.Fading.Current > renderState.Fading.Target)
							{
								renderState.Fading.Current = glm::max(renderState.Fading.Current - 0.1f * updateTime, renderState.Fading.Target);
							}
						}
						else
						{
							if (renderState.Fading.Current < 1.0f)
							{
								renderState.Fading.Current = glm::min(renderState.Fading.Current + 0.1f * updateTime, 1.0f);
							}
						}
					}
				}
			)
		)
	);
}

void NObjects::Render(const NRenderSettings &renderSettings)
{
	const auto view = Registry.view<NEntity::NRenderable, NEntity::NAttachment, NEntity::NRenderState, NEntity::NSkeleton>();

	const auto renderMode = MURenderState::GetRenderMode();
	switch (renderMode)
	{
	case NRenderMode::Normal:
		{
			for (auto [entity, attachment, renderState, skeleton] : view.each())
			{
				if (!renderState.Flags.Visible) continue;
				if (skeleton.SkeletonOffset == NInvalidUInt32) continue;

				NRenderConfig config = {
					.BoneOffset = skeleton.SkeletonOffset,
					.BodyOrigin = glm::vec3(0.0f, 0.0f, 0.0f),
					.BodyScale = 1.0f,
					.EnableLight = renderState.Flags.LightEnable,
					.BodyLight = renderState.BodyLight,
				};
				if (renderState.Flags.ShouldFade) config.BodyLight.a *= renderState.Fading.Current;
				MUModelRenderer::RenderBody(skeleton.Instance, attachment.Base, config);

				for (auto &[type, part] : attachment.Parts)
				{
					const auto model = part.Model;
					auto &skeletonInstance = part.IsLinked ? part.Link.Skeleton : skeleton.Instance;

					const NRenderConfig config = {
						.BoneOffset = part.IsLinked ? part.Link.SkeletonOffset : skeleton.SkeletonOffset,
						.BodyOrigin = glm::vec3(0.0f, 0.0f, 0.0f),
						.BodyScale = 1.0f,
						.EnableLight = renderState.Flags.LightEnable,
						.BodyLight = renderState.BodyLight,
					};
					MUModelRenderer::RenderBody(skeletonInstance, part.Model, config);
				}
			}

#if NEXTMU_RENDER_BBOX
			const auto bboxView = Registry.view<NEntity::NRenderable, NEntity::NRenderState, NEntity::NBoundingBoxes>();
			for (auto [entity, renderState, boundingBox] : bboxView.each())
			{
				if (!renderState.Flags.Visible) continue;
				MUBBoxRenderer::Render(boundingBox.Calculated);
			}
#endif
		}
		break;

	case NRenderMode::ShadowMap:
		{
			for (auto [entity, attachment, renderState, skeleton] : view.each())
			{
				if (!renderState.ShadowVisible[renderSettings.CurrentShadowMap]) continue;
				if (skeleton.SkeletonOffset == NInvalidUInt32) continue;

				NRenderConfig config = {
					.BoneOffset = skeleton.SkeletonOffset,
					.BodyOrigin = glm::vec3(0.0f, 0.0f, 0.0f),
					.BodyScale = 1.0f,
					.EnableLight = renderState.Flags.LightEnable,
					.BodyLight = renderState.BodyLight,
				};
				if (renderState.Flags.ShouldFade) config.BodyLight.a *= renderState.Fading.Current;
				MUModelRenderer::RenderBody(skeleton.Instance, attachment.Base, config);

				for (auto &[type, part] : attachment.Parts)
				{
					const auto model = part.Model;
					auto &skeletonInstance = part.IsLinked ? part.Link.Skeleton : skeleton.Instance;

					const NRenderConfig config = {
						.BoneOffset = part.IsLinked ? part.Link.SkeletonOffset : skeleton.SkeletonOffset,
						.BodyOrigin = glm::vec3(0.0f, 0.0f, 0.0f),
						.BodyScale = 1.0f,
						.EnableLight = renderState.Flags.LightEnable,
						.BodyLight = renderState.BodyLight,
					};
					MUModelRenderer::RenderBody(skeletonInstance, part.Model, config);
				}
			}
		}
		break;
	}
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

	NEntity::NRenderState renderState{
			.Flags = NEntity::NRenderFlags{
				.Visible = false,
				.LightEnable = object.LightEnable,
				.ShouldFade = object.ShouldFade,
			},
			.BodyLight = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
	};
	registry.emplace<NEntity::NRenderState>(
		entity,
		renderState
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

	registry.emplace<NEntity::NBoundingBoxes>(
		entity,
		NEntity::NBoundingBoxes{
			.Configured = NBoundingBox{
				.Min = object.BBoxMin,
				.Max = object.BBoxMax,
			}
		}
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