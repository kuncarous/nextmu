#include "stdafx.h"
#include "mu_bboxrenderer.h"
#include "mu_resourcesmanager.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace MUBBoxRenderer
{
	bgfx::ProgramHandle BoundingBoxProgram = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle BBoxMinUniform = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle BBoxMaxUniform = BGFX_INVALID_HANDLE;
	bgfx::VertexBufferHandle VertexBuffer = BGFX_INVALID_HANDLE;
	bgfx::VertexLayout VertexLayout;
	bgfx::IndexBufferHandle IndexBuffer = BGFX_INVALID_HANDLE;

	struct Vertex
	{
		glm::vec3 Position;
	};

	void InitializeVertexLayout()
	{
		static mu_boolean initialized = false;
		if (initialized) return;
		initialized = true;
		VertexLayout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.end();
	}

	const mu_boolean Initialize()
	{
		BoundingBoxProgram = MUResourcesManager::GetProgram("boundingbox");

		BBoxMinUniform = bgfx::createUniform("u_bboxMin", bgfx::UniformType::Vec4);
		if (bgfx::isValid(BBoxMinUniform) == false)
		{
			return false;
		}

		BBoxMaxUniform = bgfx::createUniform("u_bboxMax", bgfx::UniformType::Vec4);
		if (bgfx::isValid(BBoxMaxUniform) == false)
		{
			return false;
		}

		InitializeVertexLayout();
		const bgfx::Memory *mem = bgfx::alloc(8 * sizeof(Vertex));
		Vertex *vertices = reinterpret_cast<Vertex *>(mem->data);
		vertices[0].Position = glm::vec3(0.0f, 0.0f, 0.0f);
		vertices[1].Position = glm::vec3(1.0f, 0.0f, 0.0f);
		vertices[2].Position = glm::vec3(1.0f, 1.0f, 0.0f);
		vertices[3].Position = glm::vec3(0.0f, 1.0f, 0.0f);
		vertices[4].Position = glm::vec3(0.0f, 0.0f, 1.0f);
		vertices[5].Position = glm::vec3(1.0f, 0.0f, 1.0f);
		vertices[6].Position = glm::vec3(1.0f, 1.0f, 1.0f);
		vertices[7].Position = glm::vec3(0.0f, 1.0f, 1.0f);
		VertexBuffer = bgfx::createVertexBuffer(mem, VertexLayout);

		mem = bgfx::alloc(6 * 6 * sizeof(mu_uint16));
		mu_uint16 *indexes = reinterpret_cast<mu_uint16 *>(mem->data);
		indexes[0] = 0; indexes[1] = 1; indexes[2] = 2; indexes += 3;
		indexes[0] = 0; indexes[1] = 2; indexes[2] = 3; indexes += 3;

		indexes[0] = 4; indexes[1] = 5; indexes[2] = 6; indexes += 3;
		indexes[0] = 4; indexes[1] = 6; indexes[2] = 7; indexes += 3;

		indexes[0] = 0; indexes[1] = 4; indexes[2] = 7; indexes += 3;
		indexes[0] = 0; indexes[1] = 7; indexes[2] = 3; indexes += 3;

		indexes[0] = 0; indexes[1] = 4; indexes[2] = 5; indexes += 3;
		indexes[0] = 0; indexes[1] = 5; indexes[2] = 1; indexes += 3;

		indexes[0] = 7; indexes[1] = 6; indexes[2] = 2; indexes += 3;
		indexes[0] = 7; indexes[1] = 2; indexes[2] = 3; indexes += 3;

		indexes[0] = 1; indexes[1] = 5; indexes[2] = 6; indexes += 3;
		indexes[0] = 1; indexes[1] = 6; indexes[2] = 2; indexes += 3;
		IndexBuffer = bgfx::createIndexBuffer(mem);

		return true;
	}

	void Destroy()
	{
		if (bgfx::isValid(BBoxMinUniform))
		{
			bgfx::destroy(BBoxMinUniform);
			BBoxMinUniform = BGFX_INVALID_HANDLE;
		}

		if (bgfx::isValid(BBoxMaxUniform))
		{
			bgfx::destroy(BBoxMaxUniform);
			BBoxMaxUniform = BGFX_INVALID_HANDLE;
		}

		if (bgfx::isValid(VertexBuffer))
		{
			bgfx::destroy(VertexBuffer);
			VertexBuffer = BGFX_INVALID_HANDLE;
		}

		if (bgfx::isValid(IndexBuffer))
		{
			bgfx::destroy(IndexBuffer);
			IndexBuffer = BGFX_INVALID_HANDLE;
		}
	}

	void Render(
		NBoundingBox &bbox
	)
	{
		bgfx::setState((BGFX_STATE_DEFAULT | BGFX_STATE_BLEND_ADD) ^ (BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_CULL_CW));
		bgfx::setUniform(BBoxMinUniform, glm::value_ptr(bbox.Min));
		bgfx::setUniform(BBoxMaxUniform, glm::value_ptr(bbox.Max));
		bgfx::setVertexBuffer(0, VertexBuffer);
		bgfx::setIndexBuffer(IndexBuffer);
		bgfx::submit(0, BoundingBoxProgram);
	}
}