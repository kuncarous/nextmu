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
#include "mu_animationsmanager.h"
#include "mu_charactersmanager.h"
#include "res_items.h"
#include "res_renders.h"

NCharacters::NCharacters(const NEnvironment *environment) : Environment(environment)
{}

const mu_boolean NCharacters::Initialize()
{
	return true;
}

void NCharacters::Destroy()
{

}

void NCharacters::Update()
{
	const auto characters = this;
	const auto updateTime = MUState::GetUpdateTime();
	const auto environment = MURenderState::GetEnvironment();

	const auto view = Registry.view<
		NEntity::NRenderable
	>();

	MUThreadsManager::Run(
		std::unique_ptr<NThreadExecutorBase>(
			new (std::nothrow) NThreadExecutorIterator(
				view.begin(), view.end(),
				[characters, updateTime](const entt::entity entity) -> void {
					characters->MoveCharacter(entity);
				}
			)
		)
	);
}

void NCharacters::PreRender(const NRenderSettings &renderSettings)
{
	auto characters = this;
	const auto updateTime = MUState::GetUpdateTime();
	const auto environment = MURenderState::GetEnvironment();

	const auto view = Registry.view<
		NEntity::NRenderable,
		NEntity::NAttachment,
		NEntity::NLight,
		NEntity::NRenderState,
		NEntity::NSkeleton,
		NEntity::NPosition,
		NEntity::NAnimationsMapping,
		NEntity::NAnimation,
		NEntity::NAction,
		NEntity::NBoundingBoxes
	>();

	MUThreadsManager::Run(
		std::unique_ptr<NThreadExecutorBase>(
			new (std::nothrow) NThreadExecutorIterator(
				view.begin(), view.end(),
				[&view, environment, characters, &renderSettings, updateTime](const entt::entity entity) -> void {
					auto [attachment, light, renderState, skeleton, position, animationsMapping, animation, action, boundingBox] = view.get<
						NEntity::NAttachment,
						NEntity::NLight,
						NEntity::NRenderState,
						NEntity::NSkeleton,
						NEntity::NPosition,
						NEntity::NAnimationsMapping,
						NEntity::NAnimation,
						NEntity::NAction,
						NEntity::NBoundingBoxes
					>(entity);

					characters->SetCharacterAnimation(action.Type, position, animationsMapping, animation, attachment);

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
					model->PlayAnimation(animation.CurrentAction, animation.PriorAction, animation.CurrentFrame, animation.PriorFrame, model->GetPlaySpeed(animation.CurrentAction) * characters->GetAnimationModifier(entity, animation.ModifierType) * updateTime);

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
							position.HeadAngle
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
				}
			)
		)
	);
}

void NCharacters::Render(const NRenderSettings &renderSettings)
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

				const auto character = attachment.Character;
				if (character != nullptr)
				{
					for (const auto &attachment : character->Attachments)
					{
						MURenderState::AttachTexture(attachment.Type, attachment.Texture);
					}
				}

				const NRenderConfig config = {
					.BoneOffset = skeleton.SkeletonOffset,
					.BodyOrigin = position.Position,
					.BodyScale = 1.0f,
					.EnableLight = renderState.Flags.LightEnable,
					.BodyLight = renderState.BodyLight,
				};
				MUModelRenderer::RenderBody(skeleton.Instance, attachment.Base, config);

				mu_boolean hideBody[NEntity::MaxPartType] = {};
				for (auto &[type, part] : attachment.Parts)
				{
					const auto model = part.Model;
					auto &skeletonInstance = part.IsLinked ? part.Link.Skeleton : skeleton.Instance;
					hideBody[static_cast<mu_uint32>(type)] = model->ShouldHideBody();

					const NRenderConfig config = {
						.BoneOffset = part.IsLinked ? part.Link.SkeletonOffset : skeleton.SkeletonOffset,
						.BodyOrigin = position.Position,
						.BodyScale = 1.0f,
						.EnableLight = renderState.Flags.LightEnable,
						.BodyLight = renderState.BodyLight,
					};
					MUModelRenderer::RenderBody(skeletonInstance, part.Model, config);
				}

				if (character != nullptr)
				{
					for (mu_uint32 n = 0; n < NEntity::MaxPartType; ++n)
					{
						if (hideBody[n]) continue;
						NEntity::PartType type = static_cast<NEntity::PartType>(n);
						NModel *model = nullptr;
						switch (type)
						{
						case NEntity::PartType::Helm: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Head)]; break;
						case NEntity::PartType::Armor: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Chest)]; break;
						case NEntity::PartType::Pants: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Lower)]; break;
						case NEntity::PartType::Gloves: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Arms)]; break;
						case NEntity::PartType::Boots: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Legs)]; break;
						}
						if (model == nullptr) continue;

						auto &skeletonInstance = skeleton.Instance;
						const NRenderConfig config = {
							.BoneOffset = skeleton.SkeletonOffset,
							.BodyOrigin = position.Position,
							.BodyScale = 1.0f,
							.EnableLight = renderState.Flags.LightEnable,
							.BodyLight = renderState.BodyLight,
						};
						MUModelRenderer::RenderBody(skeletonInstance, model, config);
					}

					for (const auto &attachment : character->Attachments)
					{
						MURenderState::DetachTexture(attachment.Type);
					}
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
			for (auto [entity, position, attachment, renderState, skeleton] : view.each())
			{
				if (!renderState.ShadowVisible[renderSettings.CurrentShadowMap]) continue;
				if (skeleton.SkeletonOffset == NInvalidUInt32) continue;

				const auto character = attachment.Character;
				if (character != nullptr)
				{
					for (const auto &attachment : character->Attachments)
					{
						MURenderState::AttachTexture(attachment.Type, attachment.Texture);
					}
				}

				const NRenderConfig config = {
					.BoneOffset = skeleton.SkeletonOffset,
					.BodyOrigin = position.Position,
					.BodyScale = 1.0f,
					.EnableLight = renderState.Flags.LightEnable,
					.BodyLight = renderState.BodyLight,
				};
				MUModelRenderer::RenderBody(skeleton.Instance, attachment.Base, config);

				mu_boolean hideBody[NEntity::MaxPartType] = {};
				for (auto &[type, part] : attachment.Parts)
				{
					const auto model = part.Model;
					auto &skeletonInstance = part.IsLinked ? part.Link.Skeleton : skeleton.Instance;
					hideBody[static_cast<mu_uint32>(type)] = model->ShouldHideBody();

					const NRenderConfig config = {
						.BoneOffset = part.IsLinked ? part.Link.SkeletonOffset : skeleton.SkeletonOffset,
						.BodyOrigin = position.Position,
						.BodyScale = 1.0f,
						.EnableLight = renderState.Flags.LightEnable,
						.BodyLight = renderState.BodyLight,
					};
					MUModelRenderer::RenderBody(skeletonInstance, part.Model, config);
				}

				if (character != nullptr)
				{
					for (mu_uint32 n = 0; n < NEntity::MaxPartType; ++n)
					{
						if (hideBody[n]) continue;
						NEntity::PartType type = static_cast<NEntity::PartType>(n);
						NModel *model = nullptr;
						switch (type)
						{
						case NEntity::PartType::Helm: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Head)]; break;
						case NEntity::PartType::Armor: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Chest)]; break;
						case NEntity::PartType::Pants: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Lower)]; break;
						case NEntity::PartType::Gloves: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Arms)]; break;
						case NEntity::PartType::Boots: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Legs)]; break;
						}
						if (model == nullptr) continue;

						auto &skeletonInstance = skeleton.Instance;
						const NRenderConfig config = {
							.BoneOffset = skeleton.SkeletonOffset,
							.BodyOrigin = position.Position,
							.BodyScale = 1.0f,
							.EnableLight = renderState.Flags.LightEnable,
							.BodyLight = renderState.BodyLight,
						};
						MUModelRenderer::RenderBody(skeletonInstance, model, config);
					}

					for (const auto &attachment : character->Attachments)
					{
						MURenderState::DetachTexture(attachment.Type);
					}
				}
			}
		}
		break;
	}
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

	NEntity::NCharacterInfo info;
	info.Type = character.Type;
	if (character.Type == CharacterType::Character)
	{
		info.CharacterType = character.CharacterType;
	}
	else
	{
		info.MonsterType = character.MonsterType;
	}
	registry.emplace<NEntity::NCharacterInfo>(
		entity,
		info
	);

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

	const auto terrain = Environment->GetTerrain();
	const mu_float positionX = (static_cast<mu_float>(character.X) + 0.5f) * TerrainScale;
	const mu_float positionY = (static_cast<mu_float>(character.Y) + 0.5f) * TerrainScale;
	registry.emplace<NEntity::NPosition>(
		entity,
		NEntity::NPosition{
			.Position = glm::vec3(
				positionX,
				positionY,
				terrain->RequestHeight(positionX, positionY)
			),
			.Angle = glm::vec3(0.0f, 0.0f, character.Rotation),
		}
	);

	registry.emplace<NEntity::NAction>(
		entity,
		NEntity::NAction()
	);

	registry.emplace<NEntity::NMovement>(
		entity,
		NEntity::NMovement()
	);

	registry.emplace<NEntity::NMoveSpeed>(
		entity,
		NEntity::NMoveSpeed()
	);

	registry.emplace<NEntity::NAnimation>(
		entity,
		NEntity::NAnimation{
			.CurrentAction = 11,
			.PriorAction = 11,
		}
	);

	registry.emplace<NEntity::NModifiers>(
		entity,
		NEntity::NModifiers()
	);

	registry.emplace<NEntity::NAnimationsMapping>(
		entity,
		NEntity::NAnimationsMapping{
			.Root = MUAnimationsManager::GetAnimationsRoot(character.AnimationsId)
		}
	);

	registry.emplace<NEntity::NAttachment>(
		entity,
		NEntity::NAttachment{
			.Character = (character.Type == CharacterType::Character ? MUCharactersManager::GetConfiguration(character.CharacterType.Class, character.CharacterType.SubClass) : nullptr),
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

	ConfigureAnimationsMapping(entity);

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
	NItem *item = MUItemsManager::GetItem(static_cast<mu_uint16>(category), index);
	if (item == nullptr) return;
	NRender *render = item->Render;
	AddAttachmentPart(entity, partType, render);
}

void NCharacters::AddAttachmentPart(const entt::entity entity, const NEntity::PartType partType, NRender *render)
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

void NCharacters::ConfigureAnimationsMapping(const entt::entity entity)
{
	auto [info, animationsMapping, attachment] = Registry.get<NEntity::NCharacterInfo, NEntity::NAnimationsMapping, NEntity::NAttachment>(entity);
	if (animationsMapping.Root == nullptr) return;
	
	animationsMapping.Normal.clear();
	animationsMapping.Safezone.clear();

	NAnimationInput input;
	input.Swimming = Environment->GetTerrain()->IsSwimming();
	input.CharacterType = info.CharacterType;
	input.Sex = (attachment.Character != nullptr ? attachment.Character->Sex : NCharacterSex::Male);

	for (mu_uint32 n = 0; n < AnimationTypeMax; ++n)
	{
		const auto &id = AnimationTypeStrings[n];
		input.Action = id;
		const auto animationId = MUAnimationsManager::GetAnimation(animationsMapping.Root, input);
		if (animationId.empty() == true) continue;
		const auto index = attachment.Base->GetAnimationById(animationId);
		if (index == NInvalidUInt32) continue;
		animationsMapping.Normal.insert(std::make_pair(static_cast<NAnimationType>(n), index));
	}

	input.SafeZone = true;

	for (mu_uint32 n = 0; n < AnimationTypeMax; ++n)
	{
		const auto &id = AnimationTypeStrings[n];
		input.Action = id;
		const auto animationId = MUAnimationsManager::GetAnimation(animationsMapping.Root, input);
		if (animationId.empty() == true) continue;
		const auto index = attachment.Base->GetAnimationById(animationId);
		if (index == NInvalidUInt32) continue;
		animationsMapping.Safezone.insert(std::make_pair(static_cast<NAnimationType>(n), index));
	}
}

void NCharacters::SetCharacterAction(const entt::entity entity, NAnimationType type)
{
	auto &action = Registry.get<NEntity::NAction>(entity);
	action.Type = type;
}

void NCharacters::SetCharacterAnimation(NAnimationType type, NEntity::NPosition& position, NEntity::NAnimationsMapping &animationsMapping, NEntity::NAnimation &animation, NEntity::NAttachment &attachment)
{
	const auto terrain = Environment->GetTerrain();
	const auto attribute = terrain->GetAttribute(GetPositionFromFloat(position.Position.x), GetPositionFromFloat(position.Position.y));
	const auto safezone = (attribute & TerrainAttribute::SafeZone) != 0;

	const auto &mapping = (safezone ? animationsMapping.Safezone : animationsMapping.Normal);
	const auto iter = mapping.find(type);
	if (iter == mapping.end()) return;

	const auto action = iter->second;
	if (animation.CurrentAction == action) return;

	animation.ModifierType = attachment.Base->GetAnimationModifierType(action);
	animation.PriorAction = animation.CurrentAction;
	animation.PriorFrame = animation.CurrentFrame;
	animation.CurrentAction = action;
	animation.CurrentFrame = 0.0f;
}

const mu_float NCharacters::GetAnimationModifier(const entt::entity entity, NAnimationModifierType type) const
{
	auto &modifiers = Registry.get<NEntity::NModifiers>(entity);
	switch (type)
	{
	case NAnimationModifierType::MoveSpeed: return modifiers.Normalized.MoveSpeed;
	case NAnimationModifierType::AttackSpeed: return modifiers.Normalized.AttackSpeed;
	default: return 1.0f;
	}
}