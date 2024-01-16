#include "mu_precompiled.h"
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
	const auto objects = this;

	// Update Objects
	{
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

		for (auto &[group, fadingGroup] : FadingGroups)
		{
			fadingGroup->Fading.exchange(false, std::memory_order_relaxed);
		}

		const auto distanceToCharacter = Environment->GetController()->GetDistanceToCharacter();
		const auto nearPoint = Environment->GetController()->GetNearPoint();
		MUThreadsManager::Run(
			std::unique_ptr<NThreadExecutorBase>(
				new (std::nothrow) NThreadExecutorIterator(
					view.begin(), view.end(),
					[&view, environment, objects, &renderSettings, updateTime, distanceToCharacter, nearPoint](const entt::entity entity) -> void {
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
							glm::vec3(0.0f, 0.0f, 0.0f),
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

						auto &obb = boundingBox.OBB.Calculated;
						if (model->HasMeshes() && model->HasGlobalBBox())
						{
							const auto &globalBBox = model->GetGlobalBBox();
							obb = globalBBox.Transform(viewModel);
						}
						else
						{
							obb = boundingBox.OBB.Configured.Transform(viewModel);
						}

						auto &bbox = boundingBox.AABB.Calculated;
						bbox = NBoundingBox(obb);

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
							const auto bone = part.IsLinked ? part.Link.RenderAnimation.Bone : 0;

							if (part.IsLinked == true)
							{
								auto &link = part.Link;
								auto &animation = link.Animation;
								model->PlayAnimation(animation.CurrentAction, animation.PriorAction, animation.CurrentFrame, animation.PriorFrame, model->GetPlaySpeed(animation.CurrentAction) * updateTime);
							}

							if (model->HasMeshes() && model->HasGlobalBBox())
							{
								auto boneMatrix = skeleton.Instance.GetBone(bone);
								MixBones(viewModel, boneMatrix);

								const NOrientedBoundingBox globalBBox = model->GetGlobalBBox().Transform(boneMatrix);
								NBoundingBox tmpBBox(globalBBox);

								bbox.Min = glm::min(bbox.Min, tmpBBox.Min);
								bbox.Max = glm::max(bbox.Max, tmpBBox.Max);
								bbox.Order();
							}
						}

						const mu_boolean isVisible = Diligent::GetBoxVisibility(
							*renderSettings.Frustum,
							Diligent::BoundBox{
								.Min = Diligent::float3(bbox.Min.x, bbox.Min.z, bbox.Min.y),
								.Max = Diligent::float3(bbox.Max.x, bbox.Max.z, bbox.Max.y),
							}
						) != Diligent::BoxVisibility::Invisible;

						mu_boolean shadowVisible = false;
						if (renderSettings.ShadowFrustums != nullptr)
						{
							renderState.ShadowVisible = NInvalidUInt8;
							mu_uint32 shadowIndex = 0;
							for (shadowIndex; shadowIndex < renderSettings.ShadowFrustumsNum; ++shadowIndex)
							{
								const auto isVisible = Diligent::GetBoxVisibility(
									renderSettings.ShadowFrustums[shadowIndex],
									Diligent::BoundBox{
										.Min = Diligent::float3(bbox.Min.x, bbox.Min.z, bbox.Min.y),
										.Max = Diligent::float3(bbox.Max.x, bbox.Max.z, bbox.Max.y),
									},
									Diligent::FRUSTUM_PLANE_FLAG_OPEN_NEAR
								) != Diligent::BoxVisibility::Invisible;
								if (isVisible) break;
							}
							if (shadowIndex < renderSettings.ShadowFrustumsNum)
							{
								renderState.ShadowVisible = static_cast<mu_uint8>(shadowIndex);
								shadowVisible = true;
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

							const auto boneMatrix = skeleton.Instance.GetBone(renderAnimation.Bone);
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

						if (renderState.Fading.Group != nullptr && distanceToCharacter > 0.0f)
						{
							auto fadingGroup = renderState.Fading.Group;
							const auto &bbox = boundingBox.AABB.Calculated;
							Diligent::BoundBox dbbox;
							dbbox.Min = Diligent::float3(bbox.Min.x, bbox.Min.y, bbox.Min.z);
							dbbox.Max = Diligent::float3(bbox.Max.x, bbox.Max.y, bbox.Max.z);
							const mu_float distance = Diligent::GetPointToBoxDistance(dbbox, nearPoint);
							if (distance <= distanceToCharacter) fadingGroup->Fading = true;
						}
					}
				)
			)
		);
	}

	// Fading Update
	{
		const auto view = Registry.view<
			NEntity::NFading,
			NEntity::NRenderState
		>();

		MUThreadsManager::Run(
			std::unique_ptr<NThreadExecutorBase>(
				new (std::nothrow) NThreadExecutorIterator(
					view.begin(), view.end(),
					[&view, environment, objects, updateTime](const entt::entity entity) -> void {
						auto &renderState = view.get<
							NEntity::NRenderState
						>(entity);

						if (renderState.Fading.Group != nullptr)
						{
							auto fadingGroup = renderState.Fading.Group;
							if (fadingGroup->Fading.load(std::memory_order_relaxed))
							{
								if (renderState.Fading.Current > fadingGroup->Target)
								{
									renderState.Fading.Current = glm::max(renderState.Fading.Current - fadingGroup->Speed * updateTime, fadingGroup->Target);
								}
							}
							else
							{
								if (renderState.Fading.Current < 1.0f)
								{
									renderState.Fading.Current = glm::min(renderState.Fading.Current + fadingGroup->Speed * updateTime, 1.0f);
								}
							}
						}
					}
				)
			)
		);
	}
}

void NObjects::Render(const NRenderSettings &renderSettings)
{
	const auto view = Registry.view<NEntity::NRenderable, NEntity::NPosition, NEntity::NAttachment, NEntity::NRenderState, NEntity::NSkeleton>();

	const auto renderMode = MURenderState::GetRenderMode();
	switch (renderMode)
	{
	case NRenderMode::Normal:
		{
			for (auto [entity, position, attachment, renderState, skeleton] : view.each())
			{
				if (!renderState.Flags.Visible) continue;
				if (skeleton.SkeletonOffset == NInvalidUInt32) continue;

				NRenderConfig config = {
					.BoneOffset = skeleton.SkeletonOffset,
					.BodyOrigin = position.Position,
					.BodyScale = 1.0f,
					.EnableLight = renderState.Flags.LightEnable,
					.BodyLight = renderState.BodyLight,
				};
				if (renderState.Fading.Group != nullptr) {
					config.BodyLight.a *= renderState.Fading.Current;
				}
				MUModelRenderer::RenderBody(skeleton.Instance, attachment.Base, config);

				for (auto &[type, part] : attachment.Parts)
				{
					const auto model = part.Model;
					auto &skeletonInstance = part.IsLinked ? part.Link.Skeleton : skeleton.Instance;

					const NRenderConfig config = {
						.BoneOffset = part.IsLinked ? part.Link.SkeletonOffset : skeleton.SkeletonOffset,
						.BodyOrigin = position.Position,
						.BodyScale = 1.0f,
						.EnableLight = renderState.Flags.LightEnable,
						.BodyLight = renderState.BodyLight,
					};
					MUModelRenderer::RenderBody(skeletonInstance, part.Model, config, &part.Toggles, &part.Lights);
				}
			}

#if NEXTMU_RENDER_BBOX
			const auto bboxView = Registry.view<NEntity::NRenderable, NEntity::NRenderState, NEntity::NBoundingBoxes>();
			for (auto [entity, renderState, boundingBox] : bboxView.each())
			{
				if (!renderState.Flags.Visible) continue;
				MUBBoxRenderer::Render(boundingBox.AABB.Calculated);
			}
#endif
		}
		break;

	case NRenderMode::ShadowMap:
		{
			const auto shadowMapIndex = renderSettings.CurrentShadowMap;
			for (auto [entity, position, attachment, renderState, skeleton] : view.each())
			{
				if (renderState.ShadowVisible > shadowMapIndex) continue;
				if (skeleton.SkeletonOffset == NInvalidUInt32) continue;

				NRenderConfig config = {
					.BoneOffset = skeleton.SkeletonOffset,
					.BodyOrigin = position.Position,
					.BodyScale = 1.0f,
					.EnableLight = renderState.Flags.LightEnable,
					.BodyLight = renderState.BodyLight,
				};
				if (renderState.Fading.Group != nullptr) {
					config.BodyLight.a *= renderState.Fading.Current;
				}
				MUModelRenderer::RenderBody(skeleton.Instance, attachment.Base, config);

				for (auto &[type, part] : attachment.Parts)
				{
					const auto model = part.Model;
					auto &skeletonInstance = part.IsLinked ? part.Link.Skeleton : skeleton.Instance;

					const NRenderConfig config = {
						.BoneOffset = part.IsLinked ? part.Link.SkeletonOffset : skeleton.SkeletonOffset,
						.BodyOrigin = position.Position,
						.BodyScale = 1.0f,
						.EnableLight = renderState.Flags.LightEnable,
						.BodyLight = renderState.BodyLight,
					};
					MUModelRenderer::RenderBody(skeletonInstance, part.Model, config, &part.Toggles, &part.Lights);
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

	if (object.FadingGroup != NInvalidUInt32)
	{
		registry.emplace<NEntity::NFading>(entity);
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
			},
			.BodyLight = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
			.Fading = NEntity::NRenderFading{
				.Group = GetFadingGroup(object.FadingGroup),
				.Current = 1.0f
			}
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
			.OBB = {
				.Configured = NOrientedBoundingBox(
					NBoundingBox(object.BBoxMin, object.BBoxMax)
				)
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

void NObjects::ClearFadingGroups()
{
	FadingGroups.clear();
}

void NObjects::AddFadingGroup(const mu_uint32 group, const mu_float target, const mu_float speed)
{
	FadingGroups.emplace(group, std::make_unique<NFadingGroup>());
}

NFadingGroup *NObjects::GetFadingGroup(const mu_uint32 group)
{
	if (group == NInvalidUInt32) return nullptr;
	auto iter = FadingGroups.find(group);
	if (iter == FadingGroups.end()) return nullptr;
	return iter->second.get();
}