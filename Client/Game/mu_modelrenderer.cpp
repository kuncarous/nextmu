#include "stdafx.h"
#include "mu_modelrenderer.h"
#include "mu_skeletonmanager.h"
#include "mu_resourcesmanager.h"
#include "mu_skeletoninstance.h"
#include "mu_renderstate.h"
#include <glm/gtc/type_ptr.hpp>

bgfx::UniformHandle MUModelRenderer::TextureSampler = BGFX_INVALID_HANDLE;
bgfx::UniformHandle MUModelRenderer::Settings1Uniform = BGFX_INVALID_HANDLE;
bgfx::ProgramHandle MUModelRenderer::RenderProgram = BGFX_INVALID_HANDLE; // Temporary

const mu_boolean MUModelRenderer::Initialize()
{
	TextureSampler = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
	if (bgfx::isValid(TextureSampler) == false)
	{
		return false;
	}

	Settings1Uniform = bgfx::createUniform("u_settings1", bgfx::UniformType::Vec4);
	if (bgfx::isValid(Settings1Uniform) == false)
	{
		return false;
	}

	RenderProgram = MUResourcesManager::GetProgram("model_texture");
	if (bgfx::isValid(RenderProgram) == false)
	{
		return false;
	}

	return true;
}

void MUModelRenderer::Destroy()
{
	if (bgfx::isValid(TextureSampler))
	{
		bgfx::destroy(TextureSampler);
		TextureSampler = BGFX_INVALID_HANDLE;
	}

	if (bgfx::isValid(Settings1Uniform))
	{
		bgfx::destroy(Settings1Uniform);
		Settings1Uniform = BGFX_INVALID_HANDLE;
	}

	if (bgfx::isValid(RenderProgram))
	{
		bgfx::destroy(RenderProgram);
		RenderProgram = BGFX_INVALID_HANDLE;
	}
}

void MUModelRenderer::RenderMesh(const NModel *model, const mu_uint32 bonesOffset, const mu_uint32 meshIndex, const mu_uint32 transformCache)
{
	const auto &mesh = model->Meshes[meshIndex];
	if (mesh.VertexBuffer.Count == 0) return;

	auto terrain = MURenderState::GetTerrain();
	if (terrain == nullptr) return;

	auto &textureInfo = model->Textures[meshIndex];
	auto texture = MURenderState::GetTexture(textureInfo.Type);
	if (texture == nullptr)
	{
		texture = textureInfo.Texture.get();
	}
	if (texture == nullptr || texture->IsValid() == false) return;

	bgfx::setTransform(transformCache);
	bgfx::setTexture(0, TextureSampler, texture->GetTexture());

	if (texture->HasAlpha())
	{
		bgfx::setState(BGFX_STATE_DEFAULT | BGFX_STATE_BLEND_ALPHA);
	}
	else
	{
		bgfx::setState(BGFX_STATE_DEFAULT);
	}
	
	bgfx::setTexture(1, MUSkeletonManager::GetSampler(), MUSkeletonManager::GetTexture());
	bgfx::setTexture(2, terrain->GetLightmapSampler(), terrain->GetLightmapTexture());

	glm::vec4 settings(static_cast<mu_float>(bonesOffset), 0.0f, 0.0f, 0.0f);
	bgfx::setUniform(Settings1Uniform, glm::value_ptr(settings));

	bgfx::setVertexBuffer(0, model->VertexBuffer, mesh.VertexBuffer.Offset, mesh.VertexBuffer.Count);
	bgfx::submit(0, RenderProgram);
}

void MUModelRenderer::RenderBody(const NModel *model, const mu_uint32 bonesOffset, const glm::vec3 bodyOrigin, const mu_float bodyScale)
{
	auto terrain = MURenderState::GetTerrain();
	if (terrain == nullptr) return;

	glm::mat4 viewModel = glm::translate(
		glm::scale(glm::mat4(1.0f), glm::vec3(bodyScale)),
		bodyOrigin
	);
	mu_uint32 transformCache = bgfx::setTransform(glm::value_ptr(viewModel));

	const mu_uint32 numMeshes = static_cast<mu_uint32>(model->Meshes.size());
	for (mu_uint32 m = 0; m < numMeshes; ++m)
	{
		RenderMesh(model, bonesOffset, m, transformCache);
	}
}