#ifndef __T_PARTICLE_RENDER_H__
#define __T_PARTICLE_RENDER_H__

#pragma once

#include "mu_resizablequeue.h"
#include <glm/gtc/random.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace TParticle
{
	constexpr mu_uint32 MaxRenderCount = 10000;

#pragma pack(4)
	struct NParticleSettings
	{
		mu_float IsPremultipliedAlpha;
		mu_float IsLinear;
	};
#pragma pack()

	struct NRenderGroup
	{
		ParticleType Type;
		mu_uint32 Index;
		mu_uint32 Count;
	};

	struct NRenderBuffer
	{
		std::array<NParticleVertex, MaxRenderCount * 4> Vertices;
		std::array<mu_uint32, MaxRenderCount * 6> Indices;

		std::vector<NRenderGroup> Groups;

		mu_shader Program = NInvalidShader;
		mu_boolean RequireTransition = false;
		NFixedPipelineState FixedPipelineState;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> VertexBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> IndexBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> SettingsUniform;
		NResizableQueue<NParticleSettings> SettingsBuffer;
		std::map<NPipelineStateId, Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>> Bindings;
	};

	NEXTMU_INLINE void RenderSprite(NRenderBuffer &renderBuffer, const mu_uint32 renderGroup, const mu_uint32 renderIndex, const glm::vec3 position[4], const glm::vec4 &light, const glm::vec4 &uv)
	{
		auto &group = renderBuffer.Groups[renderGroup];
		auto *vertices = renderBuffer.Vertices.data() + renderIndex * 4;
		auto *indices = renderBuffer.Indices.data() + renderIndex * 6;

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

		const mu_uint32 vertex = (renderIndex - group.Index) * 4;
		*indices = vertex + 0; ++indices;
		*indices = vertex + 1; ++indices;
		*indices = vertex + 2; ++indices;
		*indices = vertex + 0; ++indices;
		*indices = vertex + 2; ++indices;
		*indices = vertex + 3; ++indices;
	}

	NEXTMU_INLINE void RenderBillboardSprite(NRenderBuffer &renderBuffer, const mu_uint32 renderGroup, const mu_uint32 renderIndex, const glm::mat4 &view, const glm::vec3 &position, const mu_float width, const mu_float height, const glm::vec4 &light, const glm::vec4 &uv = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f))
	{
		glm::vec3 cposition = view * glm::vec4(position.x, position.z, position.y, 1.0f);

		glm::vec3 rposition[4] = {
			{ cposition[0] - width, cposition[1] - height, cposition[2] },
			{ cposition[0] + width, cposition[1] - height, cposition[2] },
			{ cposition[0] + width, cposition[1] + height, cposition[2] },
			{ cposition[0] - width, cposition[1] + height, cposition[2] }
		};

		RenderSprite(renderBuffer, renderGroup, renderIndex, rposition, light, uv);
	}

	NEXTMU_INLINE void RenderBillboardSpriteWithRotation(NRenderBuffer &renderBuffer, const mu_uint32 renderGroup, const mu_uint32 renderIndex, const glm::mat4 &view, const glm::vec3 &position, const mu_float rotation, const mu_float width, const mu_float height, const glm::vec4 &light, const glm::vec4 &uv = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f))
	{
		glm::vec3 cposition = view * glm::vec4(position.x, position.z, position.y, 1.0f);

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

		RenderSprite(renderBuffer, renderGroup, renderIndex, rposition, light, uv);
	}
}

#endif