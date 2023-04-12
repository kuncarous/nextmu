#ifndef __T_JOINT_RENDER_H__
#define __T_JOINT_RENDER_H__

#pragma once

#include <glm/gtc/random.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace TJoint
{
	constexpr mu_uint32 MaxRenderCount = 5000 * 50;
#pragma pack(4)
	struct NRenderVertex
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

	struct NRenderGroup
	{
		JointType Type;
		mu_uint32 Index;
		mu_uint32 Count;
	};

	struct NRenderBuffer
	{
		std::array<NRenderVertex, MaxRenderCount * 4> Vertices;
		std::array<mu_uint32, MaxRenderCount * 6> Indices;

		std::vector<NRenderGroup> Groups;

		bgfx::VertexLayout Layout;
		bgfx::DynamicVertexBufferHandle VertexBuffer = BGFX_INVALID_HANDLE;
		bgfx::DynamicIndexBufferHandle IndexBuffer = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle TextureSampler = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle Program = BGFX_INVALID_HANDLE;
	};

	NEXTMU_INLINE void RenderTail(
		NRenderBuffer &renderBuffer,
		const mu_uint32 renderGroup,
		const mu_uint32 renderIndex,
		const glm::vec3 position[4],
		const glm::vec4 &light,
		const glm::vec4 &uv
	)
	{
		auto &group = renderBuffer.Groups[renderGroup];
		auto *vertices = renderBuffer.Vertices.data() + renderIndex * 4;
		auto *indices = renderBuffer.Indices.data() + renderIndex * 6;

#if NEXTMU_COMPRESSED_JOINTS == 1
		const auto packedLight = glm::packSnorm4x16(light);
#endif

		vertices->Position = position[0];
#if NEXTMU_COMPRESSED_JOINTS == 1
		vertices->Color = packedLight;
		vertices->UV = glm::packSnorm2x16(glm::vec2(uv[0], uv[1]));
#else
		vertices->Color = light;
		vertices->UV = glm::vec2(uv[0], uv[1]);
#endif
		++vertices;

		vertices->Position = position[1];
#if NEXTMU_COMPRESSED_JOINTS == 1
		vertices->Color = packedLight;
		vertices->UV = glm::packSnorm2x16(glm::vec2(uv[0], uv[3]));
#else
		vertices->Color = light;
		vertices->UV = glm::vec2(uv[0], uv[3]);
#endif
		++vertices;

		vertices->Position = position[2];
#if NEXTMU_COMPRESSED_JOINTS == 1
		vertices->Color = packedLight;
		vertices->UV = glm::packSnorm2x16(glm::vec2(uv[2], uv[3]));
#else
		vertices->Color = light;
		vertices->UV = glm::vec2(uv[2], uv[3]);
#endif
		++vertices;

		vertices->Position = position[3];
#if NEXTMU_COMPRESSED_JOINTS == 1
		vertices->Color = packedLight;
		vertices->UV = glm::packSnorm2x16(glm::vec2(uv[2], uv[1]));
#else
		vertices->Color = light;
		vertices->UV = glm::vec2(uv[2], uv[1]);
#endif
		++vertices;

		const mu_uint32 vertex = (renderIndex - group.Index) * 4;
		*indices = vertex + 0; ++indices;
		*indices = vertex + 1; ++indices;
		*indices = vertex + 2; ++indices;
		*indices = vertex + 0; ++indices;
		*indices = vertex + 2; ++indices;
		*indices = vertex + 3; ++indices;
	}
}

#endif