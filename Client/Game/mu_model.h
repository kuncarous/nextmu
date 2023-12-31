#ifndef __MU_MODEL_H__
#define __MU_MODEL_H__

#pragma once

#include "mu_model_mesh.h"
#include "mu_model_skeleton.h"
#include "t_textureattachments.h"

class NGraphicsTexture;
typedef std::vector<NVirtualMesh> NVirtualMeshVector;
typedef std::vector<mu_boolean> NRenderVirtualMeshToggle;
typedef std::vector<mu_uint32> NRenderVirtualMeshLightIndex;

constexpr mu_uint32 MaxBones = 200;

struct NModelTexture
{
	NTextureAttachmentType Type = NInvalidAttachment;
	std::unique_ptr<NGraphicsTexture> Texture;
};

class NModel
{
public:
	~NModel();

	const mu_boolean Load(const mu_utf8string id, mu_utf8string path);

private:
	const mu_boolean LoadModel(mu_utf8string path);
	void LoadBoundingBoxes(mu_utf8string path);
	const mu_boolean LoadTextures(const mu_utf8string path, const nlohmann::json &document);
	const mu_boolean GenerateBuffers();

	void CalculateBoundingBoxes();

public:
	const mu_boolean PlayAnimation(
		mu_uint16 &CurrentAction,
		mu_uint16 &PriorAction,
		mu_float &CurrentFrame,
		mu_float &PriorFrame,
		const mu_float PlaySpeed
	) const;

	NRenderVirtualMeshToggle GenerateVirtualMeshToggle(const NMeshRenderConditionInput &input);
	NRenderVirtualMeshLightIndex GenerateVirtualMeshLightIndex(const NMeshRenderConditionInput &input);

public:
	NEXTMU_INLINE const mu_boolean HasMeshes() const
	{
		return !Meshes.empty();
	}

	NEXTMU_INLINE const mu_boolean ShouldHideBody() const
	{
		return HideBody;
	}

	NEXTMU_INLINE const mu_boolean HasGlobalBBox() const
	{
		return BBoxes.Valid;
	}

	NEXTMU_INLINE const NOrientedBoundingBox &GetGlobalBBox() const
	{
		return BBoxes.Global;
	}

	NEXTMU_INLINE const mu_float GetPlaySpeed(const mu_uint32 index) const
	{
		return Animations[index].PlaySpeed;
	}

	NEXTMU_INLINE const mu_uint32 GetBoneById(const mu_utf8string id) const
	{
		auto iter = BonesById.find(id);
		if (iter == BonesById.end()) return NInvalidUInt32;
		return iter->second;
	}

	NEXTMU_INLINE const mu_utf8string GetAnimationId(const mu_uint32 index) const
	{
		return Animations[index].Id;
	}

	NEXTMU_INLINE const mu_uint32 GetAnimationById(const mu_utf8string id) const
	{
		auto iter = AnimationsById.find(id);
		if (iter == AnimationsById.end()) return NInvalidUInt32;
		return iter->second;
	}

	NEXTMU_INLINE const NAnimationModifierType GetAnimationModifierType(const mu_uint32 index) const
	{
		return Animations[index].Modifier;
	}

public:
	friend class NSkeletonInstance;
	friend class MUModelRenderer;

	Diligent::RefCntAutoPtr<Diligent::IBuffer> VertexBuffer;
	std::vector<NModelTexture> Textures;

	NVirtualMeshVector VirtualMeshes;
	std::vector<NMesh> Meshes;
	std::vector<mu_utf8string> BoneName; // Per Bone (separated for better cache ratio)
	std::vector<NBoneInfo> BoneInfo; // Per Bone (separated for better cache ratio)
	std::vector<NAnimation> Animations; // Per Animation (Action) (structured like this to get a better cache ratio)
	std::vector<NBoundingBoxWithValidation> BoundingBoxes; // Per Bone
	NModelBoundingBoxes BBoxes;

	std::map<mu_utf8string, mu_uint32> BonesById;
	std::map<mu_utf8string, mu_uint32> AnimationsById;

	mu_utf8string Id;
	mu_boolean HideBody = true;
	mu_int16 BoneHead = NInvalidInt16;
	mu_float BodyHeight = 0.0f;
};

#endif