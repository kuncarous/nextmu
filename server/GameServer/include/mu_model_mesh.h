#ifndef __MU_MODEL_MESH_H__
#define __MU_MODEL_MESH_H__

#pragma once

#include <array>
#include <vector>

struct NVertex
{
	mu_int16 Node = NInvalidInt16;
	glm::vec3 Position = glm::vec3();
};

struct NNormal
{
	mu_int16 Node = NInvalidInt16;
	glm::vec3 Normal = glm::vec3();
};

struct NTexCoord
{
	glm::vec2 UV = glm::vec2();
};

struct NTriangle
{
	std::array<mu_int16, 3> Vertices;
	std::array<mu_int16, 3> Normals;
	std::array<mu_int16, 3> TexCoords;
};

class NMesh
{
public:
	std::vector<NVertex> Vertices;
	std::vector<NNormal> Normals;
	std::vector<NTexCoord> TexCoords;
	std::vector<NTriangle> Triangles;
};

#endif
