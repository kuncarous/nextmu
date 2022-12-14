#ifndef __MU_MODEL_H__
#define __MU_MODEL_H__

#pragma once

#include "mu_model_mesh.h"
#include "mu_model_skeleton.h"

constexpr mu_uint32 MaxBones = 200;

enum class TextureType : mu_uint8
{
	Direct, // It is an allocated texture
	Skin, // It should use the attached skin texture
	Hide, // It should use the attached hide texture
	Hair, // It should use the attached hair texture
};

struct NModelTexture
{
	mu_boolean Valid = false;
	TextureType Type = TextureType::Direct;
	mu_boolean HasAlpha = false;
	bgfx::TextureHandle Texture = BGFX_INVALID_HANDLE;
};

class NModel
{
public:
	~NModel();

	const mu_boolean Load(const mu_utf8string id, mu_utf8string path);

private:
	const mu_boolean LoadModel(mu_utf8string path);
	const mu_boolean LoadTextures(const mu_utf8string path, const nlohmann::json &document);
	const mu_boolean GenerateBuffers();

	void CalculateBoundingBoxes();

public:
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
	std::vector<NBoneBoundingBox> BoundingBoxes; // Per Bone

	mu_utf8string Id;
	mu_int16 BoneHead = NInvalidInt16;
	mu_float BodyHeight = 0.0f;
	mu_float PlaySpeed = 1.0f;
};

#endif