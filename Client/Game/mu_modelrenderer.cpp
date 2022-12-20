#include "stdafx.h"
#include "mu_modelrenderer.h"
#include "mu_skeletonmanager.h"
#include "mu_skeletoninstance.h"
#include "mu_renderstate.h"
#include "mu_resourcesmanager.h"
#include <glm/gtc/type_ptr.hpp>

bgfx::UniformHandle MUModelRenderer::TextureSampler = BGFX_INVALID_HANDLE;
bgfx::UniformHandle MUModelRenderer::LightPositionUniform = BGFX_INVALID_HANDLE;
bgfx::UniformHandle MUModelRenderer::Settings1Uniform = BGFX_INVALID_HANDLE;
bgfx::UniformHandle MUModelRenderer::BodyLightUniform = BGFX_INVALID_HANDLE;

const mu_boolean MUModelRenderer::Initialize()
{
	TextureSampler = bgfx::createUniform("s_texture", bgfx::UniformType::Sampler);
	if (bgfx::isValid(TextureSampler) == false)
	{
		return false;
	}

	LightPositionUniform = bgfx::createUniform("u_lightPosition", bgfx::UniformType::Vec4);
	if (bgfx::isValid(LightPositionUniform) == false)
	{
		return false;
	}

	Settings1Uniform = bgfx::createUniform("u_settings1", bgfx::UniformType::Vec4);
	if (bgfx::isValid(Settings1Uniform) == false)
	{
		return false;
	}

	BodyLightUniform = bgfx::createUniform("u_bodyLight", bgfx::UniformType::Vec4);
	if (bgfx::isValid(BodyLightUniform) == false)
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

	if (bgfx::isValid(LightPositionUniform))
	{
		bgfx::destroy(LightPositionUniform);
		LightPositionUniform = BGFX_INVALID_HANDLE;
	}

	if (bgfx::isValid(Settings1Uniform))
	{
		bgfx::destroy(Settings1Uniform);
		Settings1Uniform = BGFX_INVALID_HANDLE;
	}

	if (bgfx::isValid(BodyLightUniform))
	{
		bgfx::destroy(BodyLightUniform);
		BodyLightUniform = BGFX_INVALID_HANDLE;
	}
}

void MUModelRenderer::RenderMesh(
	const NModel *model,
	const mu_uint32 meshIndex,
	const NRenderConfig &config,
	const mu_uint32 transformCache
)
{
	const auto &mesh = model->Meshes[meshIndex];
	if (mesh.VertexBuffer.Count == 0) return;

	auto terrain = MURenderState::GetTerrain();
	if (terrain == nullptr) return;

	const auto &settings = mesh.Settings;
	auto &textureInfo = model->Textures[meshIndex];
	auto texture = settings.Texture;
	if (texture == nullptr)
		texture = MURenderState::GetTexture(textureInfo.Type);
	if (texture == nullptr)
		texture = textureInfo.Texture.get();
	if (texture == nullptr || texture->IsValid() == false) return;

	bgfx::setTransform(transformCache);
	bgfx::setTexture(0, TextureSampler, texture->GetTexture());

	if (texture->HasAlpha() || config.BodyLight[3] < 1.0f)
	{
		bgfx::setState(settings.RenderState[ModelRenderMode::Alpha]);
	}
	else
	{
		bgfx::setState(settings.RenderState[ModelRenderMode::Normal]);
	}
	
	bgfx::setTexture(1, MUSkeletonManager::GetSampler(), MUSkeletonManager::GetTexture());
	bgfx::setTexture(2, terrain->GetLightmapSampler(), terrain->GetLightmapTexture());

	bgfx::setUniform(LightPositionUniform, glm::value_ptr(terrain->GetLightPosition()));
	glm::vec4 usettings(static_cast<mu_float>(config.BoneOffset), 0.0f, static_cast<mu_float>(config.EnableLight), 0.0f);
	bgfx::setUniform(Settings1Uniform, glm::value_ptr(usettings));
	bgfx::setUniform(BodyLightUniform, glm::value_ptr(config.BodyLight));

	bgfx::setVertexBuffer(0, model->VertexBuffer, mesh.VertexBuffer.Offset, mesh.VertexBuffer.Count);
	bgfx::submit(0, settings.Program);
}

void MUModelRenderer::RenderBody(
	const NSkeletonInstance &skeleton,
	const NModel *model,
	const NRenderConfig &config
)
{
	auto terrain = MURenderState::GetTerrain();
	if (terrain == nullptr) return;

	glm::mat4 viewModel = glm::translate(
		glm::scale(glm::mat4(1.0f), glm::vec3(config.BodyScale)),
		config.BodyOrigin
	);
	mu_uint32 transformCache = bgfx::setTransform(glm::value_ptr(viewModel));

	const mu_uint32 numMeshes = static_cast<mu_uint32>(model->Meshes.size());
	for (mu_uint32 m = 0; m < numMeshes; ++m)
	{
		RenderMesh(model, m, config, transformCache);
	}
}