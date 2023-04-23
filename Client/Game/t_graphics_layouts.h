#ifndef __T_GRAPHICS_LAYOUTS_H__
#define __T_GRAPHICS_LAYOUTS_H__

#pragma once

#pragma pack(4)
struct NMeshVertex
{
	glm::vec3 Position;
#if NEXTMU_COMPRESSED_MESHS
	mu_uint32 Normal[2];
	mu_uint32 TexCoords;
#else
	glm::vec3 Normal;
	glm::vec2 TexCoords;
#endif
	mu_uint8 Bone[2];
};
#pragma pack()

#pragma pack(4)
struct NBBoxVertex
{
	glm::vec3 Position;
};
#pragma pack()

#pragma pack(4)
struct NTerrainVertex
{
	mu_uint8 x, y;
	mu_uint8 rx, ry;
};
#pragma pack()

#pragma pack(4)
struct NJointVertex
{
	glm::vec3 Position;
#if NEXTMU_COMPRESSED_JOINTS == 1
	mu_uint64 Color;
	mu_uint32 UV;
#else
	glm::vec4 Color;
	glm::vec2 UV;
#endif
};
#pragma pack()

#pragma pack(4)
struct NParticleVertex
{
	glm::vec3 Position;
#if NEXTMU_COMPRESSED_PARTICLES == 1
	mu_uint64 Color;
	mu_uint32 UV;
#else
	glm::vec4 Color;
	glm::vec2 UV;
#endif
};
#pragma pack()

void CreateInputLayouts();
Diligent::InputLayoutDesc GetInputLayout(const mu_utf8string id);

#endif