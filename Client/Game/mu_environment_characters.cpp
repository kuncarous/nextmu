#include "stdafx.h"
#include "mu_environment_characters.h"
#include "mu_environment.h"
#include "mu_entity.h"
#include "mu_camera.h"
#include "mu_modelrenderer.h"
#include "mu_bboxrenderer.h"
#include "mu_state.h"
#include "mu_renderstate.h"
#include "mu_threadsmanager.h"
#include "mu_resourcesmanager.h"
#include "res_items.h"
#include "res_renders.h"

const mu_boolean NCharacters::Initialize()
{
	return true;
}

void NCharacters::Destroy()
{

}

void NCharacters::Update()
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
		NEntity::NBoundingBoxes
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
					model->PlayAnimation(animation.CurrentAction, animation.PriorAction, animation.CurrentFrame, animation.PriorFrame, model->GetPlaySpeed() * updateTime);
					
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
							model->PlayAnimation(animation.CurrentAction, animation.PriorAction, animation.CurrentFrame, animation.PriorFrame, model->GetPlaySpeed() * updateTime);
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
						}
					}

					renderState.Flags.Visible = frustum->IsBoxVisible(bbox.Min, bbox.Max);

					if (!renderState.Flags.Visible) return;

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
				}
			)
		)
	);
}

void NCharacters::Render()
{
	const auto view = Registry.view<NEntity::NRenderable, NEntity::NAttachment, NEntity::NRenderState, NEntity::NSkeleton>();
	for (auto [entity, attachment, renderState, skeleton] : view.each())
	{
		if (!renderState.Flags.Visible) continue;
		if (skeleton.SkeletonOffset == NInvalidUInt32) continue;

		MURenderState::AttachTexture(TextureAttachment::Skin, attachment.Skin);

		const NRenderConfig config = {
			.BoneOffset = skeleton.SkeletonOffset,
			.BodyOrigin = glm::vec3(0.0f, 0.0f, 0.0f),
			.BodyScale = 1.0f,
			.EnableLight = renderState.Flags.LightEnable,
			.BodyLight = renderState.BodyLight,
		};
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

		MURenderState::DetachTexture(TextureAttachment::Skin);
	}

#if RENDER_BBOX
	const auto bboxView = Objects.view<NEntity::NRenderable, NEntity::NRenderState, NEntity::NBoundingBoxes>();
	for (auto [entity, renderState, boundingBox] : bboxView.each())
	{
		if (!renderState.Flags.Visible) continue;
		MUBBoxRenderer::Render(boundingBox.Calculated);
	}
#endif
}

void NCharacters::Clear()
{
	Registry.clear();
}

const entt::entity NCharacters::AddOrFind(
	const TCharacter::Settings character
)
{
	auto iter = RegistryMap.find(character.Key);
	if (iter != RegistryMap.end()) return iter->second;

	auto &registry = Registry;
	const auto entity = registry.create();
	RegistryMap.insert(std::pair(character.Key, entity));

	registry.emplace<NEntity::NIdentifier>(
		entity,
		character.Key
	);

	registry.emplace<NEntity::NRenderable>(entity);

	registry.emplace<NEntity::NLight>(
		entity,
		NEntity::NLight{
			.Mode = EntityLightMode::Terrain,
			.Settings = NEntity::NLightSettings{
				.Terrain = NEntity::NTerrainLight{
					.Color = glm::vec3(1.0f, 1.0f, 1.0f),
					.Intensity = 1.0f,
					.PrimaryLight = false,
				}
			}
		}
	);

	registry.emplace<NEntity::NRenderState>(
		entity,
		NEntity::NRenderState{}
	);

	registry.emplace<NEntity::NSkeleton>(
		entity,
		NEntity::NSkeleton{}
	);

	registry.emplace<NEntity::NPosition>(
		entity,
		NEntity::NPosition{
			.Position = glm::vec3(
				(static_cast<mu_float>(character.X) + 0.5f) * TerrainScale,
				(static_cast<mu_float>(character.Y) + 0.5f) * TerrainScale,
				300.0f
			),
			.Angle = glm::vec3(0.0f, 0.0f, character.Rotation),
		}
	);

	registry.emplace<NEntity::NAnimation>(
		entity,
		NEntity::NAnimation{
			.CurrentAction = 4,
			.PriorAction = 4,
		}
	);

	registry.emplace<NEntity::NAttachment>(
		entity,
		NEntity::NAttachment{
			.Skin = MUResourcesManager::GetTexture("dk_skin"),
			.Base = MUResourcesManager::GetModel("player_ani"),
		}
	);

	registry.emplace<NEntity::NBoundingBoxes>(
		entity,
		NEntity::NBoundingBoxes{
			.Configured = NBoundingBox{
				.Min = glm::vec3(-60.0f, -60.0f, 0.0f),
				.Max = glm::vec3(40.0f, 40.0f, 120.0f),
			}
		}
	);

	/*
		TO DO : remove this and configure it based on the character equipment
	*/
	AddAttachmentPart(entity, NEntity::PartType::Head, MURendersManager::GetRender("dk_head"));
	AddAttachmentPartFromItem(entity, NEntity::PartType::Helm, NItemCategory::Helm, 0);
	AddAttachmentPartFromItem(entity, NEntity::PartType::Armor, NItemCategory::Armor, 0);
	AddAttachmentPartFromItem(entity, NEntity::PartType::Pants, NItemCategory::Pants, 0);
	AddAttachmentPartFromItem(entity, NEntity::PartType::Gloves, NItemCategory::Gloves, 0);
	AddAttachmentPartFromItem(entity, NEntity::PartType::Boots, NItemCategory::Boots, 0);
	AddAttachmentPartFromItem(entity, NEntity::PartType::ItemLeft, NItemCategory::Axes, 0);
	AddAttachmentPartFromItem(entity, NEntity::PartType::ItemRight, NItemCategory::Axes, 0);

	return entity;
}

void NCharacters::Remove(const entt::entity entity)
{
	if (Registry.valid(entity) == false) return;
	const auto &identifier = Registry.get<NEntity::NIdentifier>(entity);
	RegistryMap.erase(identifier.Key);
	Registry.release(entity);
}

void NCharacters::ClearAttachmentParts(const entt::entity entity)
{
	auto &attachment = Registry.get<NEntity::NAttachment>(entity);
	attachment.Parts.clear();
}

void NCharacters::AddAttachmentPartFromItem(const entt::entity entity, const NEntity::PartType partType, const NItemCategory category, const mu_uint16 index)
{
	const NItem *item = MUItemsManager::GetItem(static_cast<mu_uint16>(category), index);
	if (item == nullptr) return;
	const NRender *render = item->Render;
	AddAttachmentPart(entity, partType, render);
}

void NCharacters::AddAttachmentPart(const entt::entity entity, const NEntity::PartType partType, const NRender *render)
{
	NEntity::NRenderPart part;
	part.Type = partType;
	part.Model = render->Model;
	part.IsLinked = render->IsLinked;

	auto &attachment = Registry.get<NEntity::NAttachment>(entity);

	if (render->IsLinked)
	{
		const NModel *model = attachment.Base;
		const auto &animation = Registry.get<NEntity::NAnimation>(entity);
		const auto animationId = model->GetAnimationId(animation.CurrentAction);
		const auto partTypeId = NEntity::GetPartTypeId(partType);
		const auto renderAnimation = render->GetAnimationById(animationId);
		const auto renderAttachment = renderAnimation->GetAttachmentById(partTypeId);

		auto &link = part.Link;
		link.Render = render;
		link.RenderAnimation = *renderAnimation;
		link.Bone = model->GetBoneById(renderAttachment->Bone);
	}

	attachment.Parts.insert(std::make_pair(partType, part));
}

void NCharacters::RemoveAttachmentPart(const entt::entity entity, const NEntity::PartType partType)
{
	auto &attachment = Registry.get<NEntity::NAttachment>(entity);
	attachment.Parts.erase(partType);
}