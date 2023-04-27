#include "stdafx.h"
#include "mu_modelrenderer.h"
#include "mu_skeletonmanager.h"
#include "mu_skeletoninstance.h"
#include "mu_config.h"
#include "mu_graphics.h"
#include "mu_renderstate.h"
#include "mu_resourcesmanager.h"
#include "mu_resizablequeue.h"
#include <glm/gtc/type_ptr.hpp>
#include <MapHelper.hpp>

#pragma pack(4)
struct NModelSettings
{
	glm::vec4 LightPosition;
	glm::vec4 BodyLight;
	mu_float BoneOffset;
	mu_float NormalScale;
	mu_float EnableLight;
	mu_float AlphaTest;
	mu_float PremultiplyAlpha;
	mu_float Dummy1;
	mu_float Dummy2;
	mu_float Dummy3;
};
#pragma pack()

Diligent::RefCntAutoPtr<Diligent::IBuffer> ModelViewUniform;
Diligent::RefCntAutoPtr<Diligent::IBuffer> ModelSettingsUniform;
NResizableQueue<glm::mat4> ModelViewBuffer;
NResizableQueue<NModelSettings> ModelSettingsBuffer;

const mu_boolean MUModelRenderer::Initialize()
{
	const auto device = MUGraphics::GetDevice();

	// Model View Projection
	{
		Diligent::BufferDesc bufferDesc;
		bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
		bufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		bufferDesc.Size = sizeof(glm::mat4);

		Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
		device->CreateBuffer(bufferDesc, nullptr, &buffer);
		if (buffer == nullptr)
		{
			return false;
		}

		ModelViewUniform = buffer;
	}

	// Model Settings
	{
		Diligent::BufferDesc bufferDesc;
		bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
		bufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		bufferDesc.Size = sizeof(NModelSettings);

		Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
		device->CreateBuffer(bufferDesc, nullptr, &buffer);
		if (buffer == nullptr)
		{
			return false;
		}

		ModelSettingsUniform = buffer;
	}

	return true;
}

void MUModelRenderer::Destroy()
{
	ModelViewUniform.Release();
	ModelSettingsUniform.Release();
}

void MUModelRenderer::Reset()
{
	ModelViewBuffer.Reset();
	ModelSettingsBuffer.Reset();
}

void MUModelRenderer::RenderMesh(
	NModel *model,
	const mu_uint32 meshIndex,
	const NRenderConfig &config,
	const glm::mat4 modelViewProj,
	const NMeshRenderSettings *settings
)
{
	const auto &mesh = model->Meshes[meshIndex];
	if (mesh.VertexBuffer.Count == 0) return;

	auto terrain = MURenderState::GetTerrain();
	if (terrain == nullptr) return;

	if (!settings) settings = &mesh.Settings;
	auto &textureInfo = model->Textures[meshIndex];
	auto texture = settings->Texture;
	if (texture == nullptr)
		texture = MURenderState::GetTexture(textureInfo.Type);
	if (texture == nullptr)
		texture = textureInfo.Texture.get();
	if (texture == nullptr || texture->IsValid() == false) return;

	const auto &renderTargetDesc = MUGraphics::GetRenderTargetDesc();

	const auto renderMode = MURenderState::GetRenderMode();
	NFixedPipelineState fixedState = {
		.CombinedShader = renderMode == NRenderMode::Normal ? settings->Program : settings->ShadowProgram,
		.RTVFormat = renderTargetDesc.ColorFormat,
		.DSVFormat = renderTargetDesc.DepthStencilFormat,
	};

	const mu_boolean isPostAlphaRendering = texture->HasAlpha() || config.BodyLight[3] < 1.0f;
	const NDynamicPipelineState *dynamicState;
	if (isPostAlphaRendering)
	{
		dynamicState = renderMode != NRenderMode::ShadowMap ? &settings->RenderState[ModelRenderMode::Alpha] : &settings->ShadowRenderState[ModelRenderMode::Alpha];
	}
	else
	{
		dynamicState = renderMode != NRenderMode::ShadowMap ? &settings->RenderState[ModelRenderMode::Normal] : &settings->ShadowRenderState[ModelRenderMode::Normal];
	}

	auto pipelineState = GetPipelineState(fixedState, *dynamicState);
	if (pipelineState->StaticInitialized == false)
	{
		auto variable = pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "cbCameraAttribs");
		if (variable) variable->Set(MURenderState::GetCameraUniform());
		variable = pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "cbLightAttribs");
		if (variable) variable->Set(MURenderState::GetLightUniform());
		variable = pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "cbLightAttribs");
		if (variable) variable->Set(MURenderState::GetLightUniform());
		variable = pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "ModelViewProj");
		if (variable) variable->Set(ModelViewUniform);
		variable = pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_SkeletonTexture");
		if (variable) variable->Set(MUSkeletonManager::GetTexture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
		variable = pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "ModelSettings");
		if (variable) variable->Set(ModelSettingsUniform);
		variable = pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "ModelSettings");
		if (variable) variable->Set(ModelSettingsUniform);
		pipelineState->StaticInitialized = true;
	}

	auto shadowMap = MURenderState::GetShadowMap();
	NShaderResourcesBinding *binding;
	if (renderMode == NRenderMode::Normal && shadowMap != nullptr)
	{
		NResourceId resourceIds[2] = { texture->GetId(), MURenderState::GetShadowResourceId() };
		binding = GetShaderBinding(pipelineState, mu_countof(resourceIds), resourceIds);
	}
	else
	{
		NResourceId resourceIds[1] = { texture->GetId() };
		binding = GetShaderBinding(pipelineState, mu_countof(resourceIds), resourceIds);
	}

	if (binding->Initialized == false)
	{
		if (shadowMap != nullptr)
		{
			if (MURenderState::GetShadowMode() == NShadowMode::PCF)
			{
				auto variable = binding->Binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_tex2DShadowMap");
				if (variable) variable->Set(shadowMap->GetSRV());
			}
			else
			{
				auto variable = binding->Binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_tex2DFilterableShadowMap");
				if (variable) variable->Set(shadowMap->GetFilterableSRV());
			}
		}
		auto variable = binding->Binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture");
		if (variable) variable->Set(texture->GetTexture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
		binding->Initialized = true;
	}

	const auto renderManager = MUGraphics::GetRenderManager();

	// Update Model View
	{
		auto uniform = ModelViewBuffer.Allocate();
		*uniform = glm::transpose(modelViewProj);
		renderManager->UpdateBufferWithMap(
			RUpdateBufferWithMap{
				.ShouldReleaseMemory = false,
				.Buffer = ModelViewUniform,
				.Data = uniform,
				.Size = sizeof(Diligent::float4x4),
				.MapType = Diligent::MAP_WRITE,
				.MapFlags = Diligent::MAP_FLAG_DISCARD,
			}
		);
	}

	// Update Model Settings
	{
		auto uniform = ModelSettingsBuffer.Allocate();
		uniform->LightPosition = terrain->GetLightPosition();
		uniform->BodyLight = config.BodyLight;
		uniform->BoneOffset = static_cast<mu_float>(config.BoneOffset);
		uniform->NormalScale = 0.0f;
		uniform->EnableLight = static_cast<mu_float>(config.EnableLight);
		uniform->AlphaTest = settings->AlphaTest;
		uniform->PremultiplyAlpha = static_cast<mu_float>(
			(
				!texture->HasAlpha() &&
				dynamicState->SrcBlend != Diligent::BLEND_FACTOR_UNDEFINED
			) ||
			(
				texture->HasAlpha() &&
				!(
					dynamicState->SrcBlend == Diligent::BLEND_FACTOR_SRC_ALPHA ||
					dynamicState->SrcBlend == Diligent::BLEND_FACTOR_SRC_ALPHA_SAT
				)
			)
		);
		renderManager->UpdateBufferWithMap(
			RUpdateBufferWithMap{
				.ShouldReleaseMemory = false,
				.Buffer = ModelSettingsUniform,
				.Data = uniform,
				.Size = sizeof(NModelSettings),
				.MapType = Diligent::MAP_WRITE,
				.MapFlags = Diligent::MAP_FLAG_DISCARD,
			}
		);
	}

	renderManager->SetPipelineState(pipelineState);
	renderManager->SetVertexBuffer(
		RSetVertexBuffer{
			.StartSlot = 0,
			.Buffer = model->VertexBuffer.RawPtr(),
			.Offset = 0,
			.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY,
			.Flags = Diligent::SET_VERTEX_BUFFERS_FLAG_NONE,
		}
	);
	renderManager->CommitShaderResources(
		RCommitShaderResources{
			.ShaderResourceBinding = binding,
		}
	);

	renderManager->Draw(
		RDraw{
			.Attribs = Diligent::DrawAttribs(mesh.VertexBuffer.Count, Diligent::DRAW_FLAG_VERIFY_ALL, 1, mesh.VertexBuffer.Offset)
		},
		RCommandListInfo{
			.Type = NDrawOrderType::Classifier,
			.Classify = settings->ClassifyMode,
			.View = 0,
			.Index = static_cast<mu_uint8>(settings->ClassifyIndex),
		}
	);
}

void MUModelRenderer::RenderBody(
	const NSkeletonInstance &skeleton,
	NModel *model,
	const NRenderConfig &config
)
{
	if (model->HasMeshes() == false) return;

	auto terrain = MURenderState::GetTerrain();
	if (terrain == nullptr) return;

	glm::mat4 modelViewProj = MURenderState::GetViewProjection() * glm::translate(
		glm::scale(glm::mat4(1.0f), glm::vec3(config.BodyScale)),
		config.BodyOrigin
	);

	if (model->VirtualMeshes.size() > 0)
	{
		for (const auto &virtualMesh : model->VirtualMeshes)
		{
			RenderMesh(model, virtualMesh.Mesh, config, modelViewProj, &virtualMesh.Settings);
		}
	}
	else
	{
		const mu_uint32 numMeshes = static_cast<mu_uint32>(model->Meshes.size());
		for (mu_uint32 m = 0; m < numMeshes; ++m)
		{
			RenderMesh(model, m, config, modelViewProj);
		}
	}
}