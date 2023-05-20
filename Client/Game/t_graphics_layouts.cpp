#include "stdafx.h"
#include "t_graphics_layouts.h"
#include <cstddef>

std::map<mu_utf8string, Diligent::InputLayoutDescX> InputLayouts;

void CreateInputLayouts()
{
	// Mesh
	{
		Diligent::InputLayoutDescX inputLayout;
		inputLayout.Add(
			Diligent::LayoutElement(
				0,
				0,
				3,
				Diligent::VALUE_TYPE::VT_FLOAT32,
				true,
				offsetof(NMeshVertex, Position),
				sizeof(NMeshVertex)
			)
		);

#if NEXTMU_COMPRESSED_MESHS
		inputLayout.Add(
			Diligent::LayoutElement(
				1,
				0,
				4,
				Diligent::VALUE_TYPE::VT_INT16,
				true,
				offsetof(NMeshVertex, Normal),
				sizeof(NMeshVertex)
			)
		);
		inputLayout.Add(
			Diligent::LayoutElement(
				2,
				0,
				2,
				Diligent::VALUE_TYPE::VT_INT16,
				true,
				offsetof(NMeshVertex, TexCoords),
				sizeof(NMeshVertex)
			)
		);
#else
		inputLayout.Add(
			Diligent::LayoutElement(
				1,
				0,
				3,
				Diligent::VALUE_TYPE::VT_FLOAT32,
				true,
				offsetof(NMeshVertex, Normal),
				sizeof(NMeshVertex)
			)
		);
		inputLayout.Add(
			Diligent::LayoutElement(
				2,
				0,
				2,
				Diligent::VALUE_TYPE::VT_FLOAT32,
				true,
				offsetof(NMeshVertex, TexCoords),
				sizeof(NMeshVertex)
			)
		);
#endif

		inputLayout.Add(
			Diligent::LayoutElement(
				3,
				0,
				2,
				Diligent::VALUE_TYPE::VT_UINT8,
				false,
				offsetof(NMeshVertex, Bone),
				sizeof(NMeshVertex)
			)
		);
		inputLayout.Add(
			Diligent::LayoutElement(
				4,
				0,
				1,
				Diligent::VALUE_TYPE::VT_UINT16,
				false,
				offsetof(NMeshVertex, Vertex),
				sizeof(NMeshVertex)
			)
		);

		InputLayouts.insert(std::make_pair("mesh", inputLayout));
	}

	// Bounding Box
	{
		Diligent::InputLayoutDescX inputLayout;
		inputLayout.Add(
			Diligent::LayoutElement(
				0,
				0,
				3,
				Diligent::VALUE_TYPE::VT_FLOAT32,
				true,
				offsetof(NBBoxVertex, Position),
				sizeof(NBBoxVertex)
			)
		);

		InputLayouts.insert(std::make_pair("bbox", inputLayout));
	}

	// Terrain
	{
		Diligent::InputLayoutDescX inputLayout;
		inputLayout.Add(
			Diligent::LayoutElement(
				0,
				0,
				2,
				Diligent::VALUE_TYPE::VT_UINT8,
				false,
				offsetof(NTerrainVertex, X),
				sizeof(NTerrainVertex)
			)
		);

		inputLayout.Add(
			Diligent::LayoutElement(
				1,
				0,
				2,
				Diligent::VALUE_TYPE::VT_UINT8,
				false,
				offsetof(NTerrainVertex, RX),
				sizeof(NTerrainVertex)
			)
		);

		InputLayouts.insert(std::make_pair("terrain", inputLayout));
	}

	// Joint
	{
		Diligent::InputLayoutDescX inputLayout;
		inputLayout.Add(
			Diligent::LayoutElement(
				0,
				0,
				3,
				Diligent::VALUE_TYPE::VT_FLOAT32,
				false,
				offsetof(NJointVertex, Position),
				sizeof(NJointVertex)
			)
		);

#if NEXTMU_COMPRESSED_JOINTS == 1
		inputLayout.Add(
			Diligent::LayoutElement(
				1,
				0,
				4,
				Diligent::VALUE_TYPE::VT_INT16,
				true,
				offsetof(NJointVertex, Color),
				sizeof(NJointVertex)
			)
		);
		inputLayout.Add(
			Diligent::LayoutElement(
				2,
				0,
				2,
				Diligent::VALUE_TYPE::VT_INT16,
				true,
				offsetof(NJointVertex, UV),
				sizeof(NJointVertex)
			)
		);
#else
		inputLayout.Add(
			Diligent::LayoutElement(
				1,
				0,
				4,
				Diligent::VALUE_TYPE::VT_FLOAT32,
				false,
				offsetof(NJointVertex, Color),
				sizeof(NJointVertex)
			)
		);
		inputLayout.Add(
			Diligent::LayoutElement(
				2,
				0,
				2,
				Diligent::VALUE_TYPE::VT_FLOAT32,
				false,
				offsetof(NJointVertex, UV),
				sizeof(NJointVertex)
			)
		);
#endif

		InputLayouts.insert(std::make_pair("joint", inputLayout));
	}

	// Particle
	{
		Diligent::InputLayoutDescX inputLayout;
		inputLayout.Add(
			Diligent::LayoutElement(
				0,
				0,
				3,
				Diligent::VALUE_TYPE::VT_FLOAT32,
				false,
				offsetof(NParticleVertex, Position),
				sizeof(NParticleVertex)
			)
		);

#if NEXTMU_COMPRESSED_PARTICLES == 1
		inputLayout.Add(
			Diligent::LayoutElement(
				1,
				0,
				4,
				Diligent::VALUE_TYPE::VT_INT16,
				true,
				offsetof(NParticleVertex, Color),
				sizeof(NParticleVertex)
			)
		);
		inputLayout.Add(
			Diligent::LayoutElement(
				2,
				0,
				2,
				Diligent::VALUE_TYPE::VT_INT16,
				true,
				offsetof(NParticleVertex, UV),
				sizeof(NParticleVertex)
			)
		);
#else
		inputLayout.Add(
			Diligent::LayoutElement(
				1,
				0,
				4,
				Diligent::VALUE_TYPE::VT_FLOAT32,
				false,
				offsetof(NParticleVertex, Color),
				sizeof(NParticleVertex)
			)
		);
		inputLayout.Add(
			Diligent::LayoutElement(
				2,
				0,
				2,
				Diligent::VALUE_TYPE::VT_FLOAT32,
				false,
				offsetof(NParticleVertex, UV),
				sizeof(NParticleVertex)
			)
		);
#endif

		InputLayouts.insert(std::make_pair("particle", inputLayout));
	}
}

Diligent::InputLayoutDesc GetInputLayout(const mu_utf8string resourceId)
{
	auto iter = InputLayouts.find(resourceId);
	if (iter == InputLayouts.end()) return Diligent::InputLayoutDesc();
	return iter->second;
}