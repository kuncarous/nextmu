#ifndef __T_PARTICLE_RENDER_H__
#define __T_PARTICLE_RENDER_H__

#pragma once

#include <glm/gtc/random.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace TParticle
{
	constexpr mu_uint32 MaxRenderCount = 10000;
#pragma pack(4)
	struct NRenderVertex
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

	struct NRenderBuffer
	{
		mu_uint32 Count = 0;
		std::array<NRenderVertex, MaxRenderCount * 4> Vertices;
		std::array<mu_uint32, MaxRenderCount * 6> Indices;

		bgfx::VertexLayout Layout;
		bgfx::DynamicVertexBufferHandle VertexBuffer = BGFX_INVALID_HANDLE;
		bgfx::DynamicIndexBufferHandle IndexBuffer = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle Projection = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle TextureSampler = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle Program = BGFX_INVALID_HANDLE;
	};

	NEXTMU_INLINE void RenderSprite(NRenderBuffer &renderBuffer, mu_uint32 &vertex, const glm::vec3 position[4], const glm::vec4 &light, const glm::vec4 &uv)
	{
		const auto index = renderBuffer.Count++;
		auto *vertices = renderBuffer.Vertices.data() + index * 4;
		auto *indices = renderBuffer.Indices.data() + index * 6;

#if NEXTMU_COMPRESSED_PARTICLES == 1
		const auto packedLight = glm::packSnorm4x16(light);
#endif

		vertices->Position = position[0];
#if NEXTMU_COMPRESSED_PARTICLES == 1
		vertices->Color = packedLight;
		vertices->UV = glm::packSnorm2x16(glm::vec2(uv[0], uv[1]));
#else
		vertices->Color = light;
		vertices->UV = glm::vec2(uv[0], uv[1]);
#endif
		++vertices;

		vertices->Position = position[1];
#if NEXTMU_COMPRESSED_PARTICLES == 1
		vertices->Color = packedLight;
		vertices->UV = glm::packSnorm2x16(glm::vec2(uv[2], uv[1]));
#else
		vertices->Color = light;
		vertices->UV = glm::vec2(uv[2], uv[1]);
#endif
		++vertices;

		vertices->Position = position[2];
#if NEXTMU_COMPRESSED_PARTICLES == 1
		vertices->Color = packedLight;
		vertices->UV = glm::packSnorm2x16(glm::vec2(uv[2], uv[3]));
#else
		vertices->Color = light;
		vertices->UV = glm::vec2(uv[2], uv[3]);
#endif
		++vertices;

		vertices->Position = position[3];
#if NEXTMU_COMPRESSED_PARTICLES == 1
		vertices->Color = packedLight;
		vertices->UV = glm::packSnorm2x16(glm::vec2(uv[0], uv[3]));
#else
		vertices->Color = light;
		vertices->UV = glm::vec2(uv[0], uv[3]);
#endif
		++vertices;

		*indices = vertex + 0; ++indices;
		*indices = vertex + 1; ++indices;
		*indices = vertex + 2; ++indices;
		*indices = vertex + 0; ++indices;
		*indices = vertex + 2; ++indices;
		*indices = vertex + 3; ++indices;
		vertex += 4;
	}

	NEXTMU_INLINE void RenderBillboardSprite(NRenderBuffer &renderBuffer, mu_uint32 &vertex, cglm::mat4 view, const glm::vec3 &position, const mu_float width, const mu_float height, const glm::vec4 &light, const glm::vec4 &uv = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f))
	{
		cglm::vec3 cposition;
		cglm::glm_mat4_mulv3(view, (mu_float *)glm::value_ptr(position), 1.0f, cposition);

		glm::vec3 rposition[4] = {
			{ cposition[0] - width, cposition[1] - height, cposition[2] },
			{ cposition[0] + width, cposition[1] - height, cposition[2] },
			{ cposition[0] + width, cposition[1] + height, cposition[2] },
			{ cposition[0] - width, cposition[1] + height, cposition[2] }
		};

		RenderSprite(renderBuffer, vertex, rposition, light, uv);
	}

	NEXTMU_INLINE void RenderBillboardSpriteWithRotation(NRenderBuffer &renderBuffer, mu_uint32 &vertex, cglm::mat4 view, const glm::vec3 &position, const mu_float rotation, const mu_float width, const mu_float height, const glm::vec4 &light, const glm::vec4 &uv = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f))
	{
		cglm::vec3 cposition;
		cglm::glm_mat4_mulv3(view, (mu_float *)glm::value_ptr(position), 1.0f, cposition);

		glm::vec3 rposition[4] = {
			{ -width, -height, cposition[2] },
			{ +width, -height, cposition[2] },
			{ +width, +height, cposition[2] },
			{ -width, +height, cposition[2] }
		};

		const auto mrot = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, rotation)));
		rposition[0] = rposition[0] * mrot;
		rposition[1] = rposition[1] * mrot;
		rposition[2] = rposition[2] * mrot;
		rposition[3] = rposition[3] * mrot;

		rposition[0].x += cposition[0]; rposition[0].y += cposition[1];
		rposition[1].x += cposition[0]; rposition[1].y += cposition[1];
		rposition[2].x += cposition[0]; rposition[2].y += cposition[1];
		rposition[3].x += cposition[0]; rposition[3].y += cposition[1];

		RenderSprite(renderBuffer, vertex, rposition, light, uv);
	}
}

#endif