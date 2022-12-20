#ifndef __MU_MODEL_H__
#define __MU_MODEL_H__

#pragma once

#include "mu_model_mesh.h"
#include "mu_model_skeleton.h"
#include "mu_renderstate_enums.h"

class NTexture;

constexpr mu_uint32 MaxBones = 200;

struct NModelTexture
{
	TextureAttachment::Type Type = TextureAttachment::Normal;
	std::unique_ptr<NTexture> Texture;
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
	NEXTMU_INLINE const mu_boolean HasMeshes() const
	{
		return !Meshes.empty();
	}

	NEXTMU_INLINE const mu_boolean HasGlobalBBox() const
	{
		return BBoxes.Valid;
	}

	NEXTMU_INLINE const NBoundingBox &GetGlobalBBox() const
	{
		return BBoxes.Global;
	}

	NEXTMU_INLINE const mu_float GetPlaySpeed() const
	{
		return PlaySpeed;
	}

protected:
	friend class NSkeletonInstance;
	friend class MUModelRenderer;

	bgfx::VertexBufferHandle VertexBuffer = BGFX_INVALID_HANDLE;
	std::vector<NModelTexture> Textures;

	std::vector<NMesh> Meshes;
	std::vector<mu_utf8string> BoneName; // Per Bone (separated for better cache ratio)
	std::vector<NBoneInfo> BoneInfo; // Per Bone (separated for better cache ratio)
	std::vector<NAnimation> Animations; // Per Animation (Action) (structured like this to get a better cache ratio)
	std::vector<NBoundingBoxWithValidation> BoundingBoxes; // Per Bone
	NModelBoundingBoxes BBoxes;

	mu_utf8string Id;
	mu_int16 BoneHead = NInvalidInt16;
	mu_float BodyHeight = 0.0f;
	mu_float PlaySpeed = 1.0f;
};

#endif