#include "stdafx.h"
#include "mu_modelrenderer.h"
#include "mu_skeletonmanager.h"
#include "mu_skeletoninstance.h"
#include "mu_graphics.h"
#include "mu_renderstate.h"
#include "mu_resourcesmanager.h"
#include <glm/gtc/type_ptr.hpp>
#include <MapHelper.hpp>

std::map<NPipelineStateId, Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>> Bindings;
NFixedPipelineState PreconfiguredFixedState;
Diligent::RefCntAutoPtr<Diligent::IBuffer> ModelViewUniform;
Diligent::RefCntAutoPtr<Diligent::IBuffer> ModelSettingsUniform;

#pragma pack(4)
struct NModelSettings
{
	glm::vec4 LightPosition;
	mu_float BoneOffset;
	mu_float NormalScale;
	mu_float EnableLight;
	mu_float Dummy;
	glm::vec4 BodyLight;
};
#pragma pack()

const mu_boolean MUModelRenderer::Initialize()
{
	const auto device = MUGraphics::GetDevice();
	const auto swapchain = MUGraphics::GetSwapChain();
	const auto &swapchainDesc = swapchain->GetDesc();
	PreconfiguredFixedState.RTVFormat = swapchainDesc.ColorBufferFormat;
	PreconfiguredFixedState.DSVFormat = swapchainDesc.DepthBufferFormat;

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
	Bindings.clear();
	ModelViewUniform.Release();
	ModelSettingsUniform.Release();
}

void MUModelRenderer::RenderMesh(
	NModel *model,
	const mu_uint32 meshIndex,
	const NRenderConfig &config,
	cglm::mat4 modelViewProj,
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

	NFixedPipelineState fixedState = PreconfiguredFixedState;
	fixedState.CombinedShader = settings->Program;

	const mu_boolean isDepthRendering = texture->HasAlpha() || config.BodyLight[3] < 1.0f;
	const NDynamicPipelineState *dynamicState;
	if (isDepthRendering)
	{
		dynamicState = &settings->RenderState[ModelRenderMode::Alpha];
	}
	else
	{
		dynamicState = &settings->RenderState[ModelRenderMode::Normal];
	}

	auto pipelineState = GetPipelineState(fixedState, *dynamicState);
	auto bindingIter = Bindings.find(pipelineState->Id);
	if (bindingIter == Bindings.end())
	{
		pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "ModelViewProj")->Set(ModelViewUniform);
		pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_SkeletonTexture")->Set(MUSkeletonManager::GetTexture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
		//pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_LightTexture")->Set(terrain->GetLightmapTexture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
		pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "ModelSettings")->Set(ModelSettingsUniform);

		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> binding;
		pipelineState->Pipeline->CreateShaderResourceBinding(&binding, true);
		bindingIter = Bindings.insert(std::make_pair(pipelineState->Id, binding)).first;
	}
	auto binding = bindingIter->second.RawPtr();

	const auto renderManager = MUGraphics::GetRenderManager();

	// Update Model View
	{
		auto uniform = std::make_unique<RTemporaryBuffer>(sizeof(cglm::mat4));
		cglm::mat4 *mvp = reinterpret_cast<cglm::mat4*>(uniform->Get<float*>());
		cglm::glm_mat4_copy(modelViewProj, *mvp);
		cglm::glm_mat4_transpose(*mvp);
		renderManager->UpdateBufferWithMap(
			RUpdateBufferWithMap{
				.Buffer = ModelViewUniform,
				.Data = mvp,
				.Size = sizeof(cglm::mat4),
				.MapType = Diligent::MAP_WRITE,
				.MapFlags = Diligent::MAP_FLAG_DISCARD,
			},
			std::move(uniform)
		);
	}

	// Update Model Settings
	{
		auto uniform = std::make_unique<RTemporaryBuffer>(sizeof(NModelSettings));
		NModelSettings *settings = uniform->Get<NModelSettings*>();
		settings->LightPosition = terrain->GetLightPosition();
		settings->BoneOffset = static_cast<mu_float>(config.BoneOffset);
		settings->NormalScale = 0.0f;
		settings->EnableLight = static_cast<mu_float>(config.EnableLight);
		settings->Dummy = 0.0f;
		settings->BodyLight = config.BodyLight;
		renderManager->UpdateBufferWithMap(
			RUpdateBufferWithMap{
				.Buffer = ModelSettingsUniform,
				.Data = settings,
				.Size = sizeof(NModelSettings),
				.MapType = Diligent::MAP_WRITE,
				.MapFlags = Diligent::MAP_FLAG_DISCARD,
			},
			std::move(uniform)
		);
	}

	renderManager->SetDynamicTexture(
		RSetDynamicTexture{
			.Type = Diligent::SHADER_TYPE_PIXEL,
			.Name = "g_Texture",
			.View = texture->GetTexture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE),
			.Binding = binding,
		}
	);
	renderManager->SetPipelineState(pipelineState);
	renderManager->SetVertexBuffer(
		RSetVertexBuffer{
			.StartSlot = 0,
			.Buffer = model->VertexBuffer.RawPtr(),
			.Offset = 0,
			.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
			.Flags = Diligent::SET_VERTEX_BUFFERS_FLAG_RESET,
		}
	);
	renderManager->CommitShaderResources(
		RCommitShaderResources{
			.ShaderResourceBinding = binding,
			.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
		}
	);

	renderManager->Draw(
		RDraw{
			.Attribs = Diligent::DrawAttribs(mesh.VertexBuffer.Count, Diligent::DRAW_FLAG_VERIFY_ALL, 1, mesh.VertexBuffer.Offset)
		},
		RCommandListInfo{
			.Type = NDrawOrderType::Blend,
			.View = 0,
			.Depth = 0,
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

	cglm::mat4 viewProj;
	MURenderState::GetViewProjection(viewProj);

	glm::mat4 modelView = glm::translate(
		glm::scale(glm::mat4(1.0f), glm::vec3(config.BodyScale)),
		config.BodyOrigin
	);

	cglm::mat4 modelViewProj;
	cglm::glm_mat4_mul(viewProj, (cglm::vec4*)glm::value_ptr(modelView), modelViewProj);

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