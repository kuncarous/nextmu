#include "mu_precompiled.h"
#include "mu_environment_characters.h"
#include "mu_environment.h"
#include "mu_entity.h"
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
			new_nothrow NThreadExecutorIterator(
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
			new_nothrow NThreadExecutorIterator(
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
							position.HeadAngle
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
						for (; shadowIndex < renderSettings.ShadowFrustumsNum; ++shadowIndex)
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

				NRenderConfig config = {
					.BoneOffset = skeleton.SkeletonOffset,
					.BodyOrigin = position.Position,
					.BodyScale = 1.0f,
					.EnableLight = renderState.Flags.LightEnable,
					.BodyLight = renderState.BodyLight,
					.BlendMeshLight = 1.0f,
				};
				MUModelRenderer::RenderBody(skeleton.Instance, attachment.Base, config);

				mu_boolean hideBody[MaxPartType] = {};
				for (auto &[type, part] : attachment.Parts)
				{
					const auto model = part.Model;
					auto &skeletonInstance = part.IsLinked ? part.Link.Skeleton : skeleton.Instance;
					hideBody[static_cast<mu_uint32>(type)] = model->ShouldHideBody();

					NRenderConfig config = {
						.BoneOffset = part.IsLinked ? part.Link.SkeletonOffset : skeleton.SkeletonOffset,
						.BodyOrigin = position.Position,
						.BodyScale = 1.0f,
						.EnableLight = renderState.Flags.LightEnable,
						.BodyLight = renderState.BodyLight,
						.BlendMeshLight = 1.0f,
					};
					MUModelRenderer::RenderBody(skeletonInstance, part.Model, config, &part.Toggles, &part.Lights);
				}

				if (character != nullptr)
				{
					for (mu_uint32 n = 0; n < MaxPartType; ++n)
					{
						if (hideBody[n]) continue;
						NPartType type = static_cast<NPartType>(n);
						NModel *model = nullptr;
						switch (type)
						{
						case NPartType::Helm: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Head)]; break;
						case NPartType::Armor: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Chest)]; break;
						case NPartType::Pants: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Lower)]; break;
						case NPartType::Gloves: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Arms)]; break;
						case NPartType::Boots: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Legs)]; break;
						}
						if (model == nullptr) continue;

						auto &skeletonInstance = skeleton.Instance;
						NRenderConfig config = {
							.BoneOffset = skeleton.SkeletonOffset,
							.BodyOrigin = position.Position,
							.BodyScale = 1.0f,
							.EnableLight = renderState.Flags.LightEnable,
							.BodyLight = renderState.BodyLight,
							.BlendMeshLight = 1.0f,
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

				const auto character = attachment.Character;
				if (character != nullptr)
				{
					for (const auto &attachment : character->Attachments)
					{
						MURenderState::AttachTexture(attachment.Type, attachment.Texture);
					}
				}

				NRenderConfig config = {
					.BoneOffset = skeleton.SkeletonOffset,
					.BodyOrigin = position.Position,
					.BodyScale = 1.0f,
					.EnableLight = renderState.Flags.LightEnable,
					.BodyLight = renderState.BodyLight,
					.BlendMeshLight = 1.0f,
				};
				MUModelRenderer::RenderBody(skeleton.Instance, attachment.Base, config);

				mu_boolean hideBody[MaxPartType] = {};
				for (auto &[type, part] : attachment.Parts)
				{
					const auto model = part.Model;
					auto &skeletonInstance = part.IsLinked ? part.Link.Skeleton : skeleton.Instance;
					hideBody[static_cast<mu_uint32>(type)] = model->ShouldHideBody();

					NRenderConfig config = {
						.BoneOffset = part.IsLinked ? part.Link.SkeletonOffset : skeleton.SkeletonOffset,
						.BodyOrigin = position.Position,
						.BodyScale = 1.0f,
						.EnableLight = renderState.Flags.LightEnable,
						.BodyLight = renderState.BodyLight,
						.BlendMeshLight = 1.0f,
					};
					MUModelRenderer::RenderBody(skeletonInstance, part.Model, config, &part.Toggles, &part.Lights);
				}

				if (character != nullptr)
				{
					for (mu_uint32 n = 0; n < MaxPartType; ++n)
					{
						if (hideBody[n]) continue;
						NPartType type = static_cast<NPartType>(n);
						NModel *model = nullptr;
						switch (type)
						{
						case NPartType::Helm: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Head)]; break;
						case NPartType::Armor: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Chest)]; break;
						case NPartType::Pants: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Lower)]; break;
						case NPartType::Gloves: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Arms)]; break;
						case NPartType::Boots: model = character->Parts[static_cast<mu_uint32>(CharacterBodyPart::Legs)]; break;
						}
						if (model == nullptr) continue;

						auto &skeletonInstance = skeleton.Instance;
						NRenderConfig config = {
							.BoneOffset = skeleton.SkeletonOffset,
							.BodyOrigin = position.Position,
							.BodyScale = 1.0f,
							.EnableLight = renderState.Flags.LightEnable,
							.BodyLight = renderState.BodyLight,
							.BlendMeshLight = 1.0f,
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
	if (character.Type == NCharacterType::Character)
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
			.Character = (character.Type == NCharacterType::Character ? MUCharactersManager::GetConfiguration(character.CharacterType.Class, character.CharacterType.SubClass) : nullptr),
			.Base = MUResourcesManager::GetResourcesManager()->GetModel("player_ani"),
		}
	);
	
	registry.emplace<NEntity::NBoundingBoxes>(
		entity,
		NEntity::NBoundingBoxes{
			.OBB = {
				.Configured = NOrientedBoundingBox(
					NBoundingBox(glm::vec3(-60.0f, -60.0f, 0.0f), glm::vec3(40.0f, 40.0f, 120.0f))
				)
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
	if (Registry.orphan(entity) == false)
	{
		Registry.storage<entt::entity>().erase(entity);
	}
}

void NCharacters::ClearAttachmentParts(const entt::entity entity)
{
	auto &attachment = Registry.get<NEntity::NAttachment>(entity);
	attachment.Parts.clear();
}

void NCharacters::AddAttachmentPartFromItem(const entt::entity entity, const NPartType partType, const EItemCategory category, const mu_uint16 index)
{
	NItem *item = MUItemsManager::GetItem(static_cast<mu_uint16>(category), index);
	if (item == nullptr) return;
	NRender *render = item->Render;
	AddAttachmentPart(entity, partType, render);
}

void NCharacters::AddAttachmentPart(const entt::entity entity, const NPartType partType, NRender *render)
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
		const auto renderAnimation = render->GetAnimationById(animationId);
		const auto renderAttachment = renderAnimation->GetAttachmentByPartType(partType);

		auto &link = part.Link;
		link.Render = render;
		link.RenderAnimation.Bone = model->GetBoneById(renderAttachment->Bone);
		link.RenderAnimation.Position = renderAttachment->Position;
		link.RenderAnimation.Angle = renderAttachment->Angle;
		link.RenderAnimation.Scale = renderAttachment->Scale;
	}

	attachment.Parts.insert(std::make_pair(partType, part));
}

void NCharacters::RemoveAttachmentPart(const entt::entity entity, const NPartType partType)
{
	auto &attachment = Registry.get<NEntity::NAttachment>(entity);
	attachment.Parts.erase(partType);
}

void NCharacters::GenerateVirtualMeshToggle(const entt::entity entity)
{
	auto &attachment = Registry.get<NEntity::NAttachment>(entity);

	for (auto &[type, part] : attachment.Parts)
	{
		const auto model = part.Model;

		mu_uint32 optionsCount = static_cast<mu_uint32>(part.Options.size());

		std::vector<EItemOption> optionsType(optionsCount, optionsCount > 0 ? part.Options[0].Type : EItemOption::eMax);

		mu_float firstOptionRank = optionsCount > 0 ? ItemRankToFloat(part.Options[0].Rank) : 0.0f;
		mu_float optionsMinRank = firstOptionRank;
		mu_float optionsMaxRank = firstOptionRank;
		mu_float optionsAvgRank = firstOptionRank;

		for (mu_uint32 index = 1u; index < optionsCount; ++optionsCount)
		{
			auto &option = part.Options[index];
			optionsType[index] = option.Type;
			mu_float optionRank = ItemRankToFloat(option.Rank);
			if (optionRank < optionsMinRank) optionsMinRank = optionRank;
			if (optionRank > optionsMaxRank) optionsMaxRank = optionRank;
			optionsAvgRank += optionRank;
		}

		optionsAvgRank /= static_cast<mu_float>(optionsCount);

		NMeshRenderConditionInput input = {
			.Level = part.Level,
			.LevelByFormula = static_cast<mu_uint8>(ItemGlowToLevelFormula(part.Level)),
			.OptionsCount = static_cast<mu_uint8>(optionsCount),
			.Rank = part.Rank,
			.OptionsMinRank = ItemRankToSimpleRank(optionsMinRank),
			.OptionsMaxRank = ItemRankToSimpleRank(optionsMaxRank),
			.OptionsAvgRank = ItemRankToSimpleRank(optionsAvgRank),
		};

		part.Toggles = std::move(model->GenerateVirtualMeshToggle(input));
		part.Lights = std::move(model->GenerateVirtualMeshLightIndex(input));
	}
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

	const NModel *model = attachment.Base;
	const auto animationId = model->GetAnimationId(animation.CurrentAction);

	for (auto &[partType, part] : attachment.Parts)
	{
		if (part.IsLinked == false) continue;
		const auto *render = part.Link.Render;
		if (!render) continue;

		const auto renderAnimation = render->GetAnimationById(animationId);
		const auto renderAttachment = renderAnimation->GetAttachmentByPartType(partType);

		auto &link = part.Link;
		link.Render = render;
		link.RenderAnimation.Bone = model->GetBoneById(renderAttachment->Bone);
		link.RenderAnimation.Position = renderAttachment->Position;
		link.RenderAnimation.Angle = renderAttachment->Angle;
		link.RenderAnimation.Scale = renderAttachment->Scale;
	}
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