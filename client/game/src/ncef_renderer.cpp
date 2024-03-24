#include "mu_precompiled.h"
#include "ncef_renderer.h"
#include "mu_browsermanager.h"
#include "mu_resourcesmanager.h"
#include "mu_graphics.h"
#include "mu_textures.h"
#include "mu_renderstate.h"
#include <MapHelper.hpp>

#if NEXTMU_EMBEDDED_BROWSER == 1
#include <include/views/cef_display.h>

#pragma pack(4)
struct BrowserAttribs
{
	glm::mat4 Transform;
	glm::vec2 ScreenSize;
	mu_float IsLinear;
};
#pragma pack()

NBrowserRenderer::~NBrowserRenderer()
{
}

const mu_boolean NBrowserRenderer::Initialize()
{
	auto device = MUGraphics::GetDevice();
	const auto swapchain = MUGraphics::GetSwapChain();
	const auto &swapchainDesc = swapchain->GetDesc();
	const auto immediateContext = MUGraphics::GetImmediateContext();

	// Browser Attribs Uniform
	{
		Diligent::BufferDesc bufferDesc;
		bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
		bufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		bufferDesc.Size = sizeof(BrowserAttribs);

		Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
		device->CreateBuffer(bufferDesc, nullptr, &buffer);
		if (buffer == nullptr)
		{
			return false;
		}

		Uniform = buffer;
	}

	// Browser Vertices
	{
		std::unique_ptr<NBrowserVertex[]> memory(new_nothrow NBrowserVertex[6]);
		NBrowserVertex *vertices = reinterpret_cast<NBrowserVertex *>(memory.get());
		vertices[0].Position = glm::vec2(0.0f, 0.0f);
		vertices[1].Position = glm::vec2(1.0f, 0.0f);
		vertices[2].Position = glm::vec2(1.0f, 1.0f);
		vertices[3].Position = glm::vec2(0.0f, 0.0f);
		vertices[4].Position = glm::vec2(1.0f, 1.0f);
		vertices[5].Position = glm::vec2(0.0f, 1.0f);

		Diligent::BufferDesc bufferDesc;
		bufferDesc.Usage = Diligent::USAGE_IMMUTABLE;
		bufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
		bufferDesc.Size = sizeof(NBrowserVertex) * 6;

		Diligent::BufferData bufferData;
		bufferData.pData = memory.get();
		bufferData.DataSize = sizeof(NBrowserVertex) * 6;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
		device->CreateBuffer(bufferDesc, &bufferData, &buffer);
		if (buffer == nullptr)
		{
			return false;
		}

		Diligent::StateTransitionDesc barrier(buffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
		immediateContext->TransitionResourceStates(1, &barrier);
		VertexBuffer = buffer;
	}

	TextureId = MUTextures::GenerateTextureId();
	OnResize(static_cast<mu_int32>(swapchainDesc.Width), static_cast<mu_int32>(swapchainDesc.Height));

	return Texture != nullptr;
}

static NDynamicPipelineState DynamicState = NDynamicPipelineState{
	.CullMode = Diligent::CULL_MODE_NONE,
	.EnableScissors = false,
	.AlphaWrite = false,
	.DepthWrite = false,
	.StencilEnable = false,
	.StencilFunc = Diligent::COMPARISON_FUNC_EQUAL,
	.SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA,
	.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
	.SrcBlendAlpha = Diligent::BLEND_FACTOR_SRC_ALPHA,
	.DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
};

void NBrowserRenderer::Render()
{
	const auto swapchain = MUGraphics::GetSwapChain();
	auto *pDSV = swapchain->GetDepthBufferDSV();

	const auto immediateContext = MUGraphics::GetImmediateContext();
	immediateContext->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG | Diligent::CLEAR_STENCIL_FLAG, 1.0f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	Projection = GLMFromFloat4x4(Diligent::float4x4::OrthoOffCenter(
		0.0f,
		Width,
		Height,
		0.0f,
		0.0f, 1000.0f,
		MUGraphics::GetDeviceType() == Diligent::RENDER_DEVICE_TYPE_GL ||
		MUGraphics::GetDeviceType() == Diligent::RENDER_DEVICE_TYPE_GLES
	).Transpose());
	MURenderState::SetViewTransform(Transform, Projection, Projection, MURenderState::GetShadowView(), MURenderState::GetShadowProjection());

	if (TextureState == Diligent::RESOURCE_STATE_COPY_DEST)
	{
		Diligent::StateTransitionDesc barrier(Texture->GetTexture(), Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
		immediateContext->TransitionResourceStates(1u, &barrier);
		TextureState = Diligent::RESOURCE_STATE_SHADER_RESOURCE;
	}

	// Update Uniform
	{
		Diligent::MapHelper<BrowserAttribs> uniform(immediateContext, Uniform, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
		*uniform = BrowserAttribs {
			.Transform = Projection * Transform,
			.ScreenSize = glm::vec2(static_cast<mu_float>(Width), static_cast<mu_float>(Height)),
			.IsLinear = static_cast<mu_float>(IsLinear),
		};
	}

	const auto &renderTargetDesc = MUGraphics::GetRenderTargetDesc();

	NFixedPipelineState fixedState = {
		.CombinedShader = Shader,
		.RTVFormat = renderTargetDesc.ColorFormat,
		.DSVFormat = renderTargetDesc.DepthStencilFormat,
	};

	auto pipelineState = GetPipelineState(fixedState, DynamicState);
	if (pipelineState->StaticInitialized == false)
	{
		pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "cbBrowserAttribs")->Set(Uniform);
		pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "cbBrowserAttribs")->Set(Uniform);
		pipelineState->StaticInitialized = true;
	}

	NResourceId resourceIds[1] = { Texture->GetId() };
	auto binding = ShaderResourcesBindingManager.GetShaderBinding(pipelineState->Id, pipelineState->Pipeline, mu_countof(resourceIds), resourceIds);
	if (binding->Initialized == false)
	{
		if (Texture != nullptr) binding->Binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(Texture->GetTexture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
		binding->Initialized = true;
	}

	immediateContext->SetPipelineState(pipelineState->Pipeline);
	immediateContext->CommitShaderResources(
		binding->Binding,
		binding->ShouldTransition
		? Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
		: Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY
	);
	binding->ShouldTransition = false;

	Diligent::IBuffer *vertexBuffers[1] = { VertexBuffer };
	immediateContext->SetVertexBuffers(0, mu_countof(vertexBuffers), vertexBuffers, 0u, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

	immediateContext->Draw(
		Diligent::DrawAttribs(
			6,
			Diligent::DRAW_FLAGS::DRAW_FLAG_VERIFY_ALL
		)
	);
}

void NBrowserRenderer::OnResize(mu_int32 w, mu_int32 h)
{
	if (w == Width && h == Height) return;
	if (Texture != nullptr) Texture.reset();

	auto device = MUGraphics::GetDevice();

	const mu_uint32 width = static_cast<mu_uint32>(w);
	const mu_uint32 height = static_cast<mu_uint32>(h);
	const mu_uint32 bitmapSize = width * height * 4;
	std::unique_ptr<mu_uint8[]> bitmapBuffer(new_nothrow mu_uint8[bitmapSize]);
	mu_memset(bitmapBuffer.get(), 0xFF, bitmapSize);

	std::vector<Diligent::TextureSubResData> subresources;
	Diligent::TextureSubResData subresource;
	subresource.pData = bitmapBuffer.get();
	subresource.Stride = width * 4;
	subresources.push_back(subresource);

	Diligent::TextureDesc textureDesc;
#if NEXTMU_COMPILE_DEBUG == 1
	textureDesc.Name = "browser_texture";
#endif
	textureDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.Format = Diligent::TEX_FORMAT_BGRA8_UNORM;
	textureDesc.Usage = Diligent::USAGE_DEFAULT;
	textureDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
	textureDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;

	Diligent::TextureData textureData(subresources.data(), static_cast<mu_uint32>(subresources.size()));
	Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
	device->CreateTexture(textureDesc, &textureData, &texture);
	if (texture == nullptr)
	{
		return;
	}

	const auto immediateContext = MUGraphics::GetImmediateContext();
	Diligent::StateTransitionDesc barrier(texture, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
	immediateContext->TransitionResourceStates(1u, &barrier);
	texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE)->SetSampler(GetTextureSampler(MUTextures::CalculateSamplerFlags("nearest", "clamp"))->Sampler);

	MUGraphics::IncreaseTransactions();
	MUGraphics::CheckIfRequireFlushContext();

	Width = w;
	Height = h;
	TextureState = Diligent::RESOURCE_STATE_COPY_DEST;
	Texture = std::make_unique<NGraphicsTexture>(TextureId, texture, width, height, true);
}

void NBrowserRenderer::ReloadShaders()
{
	Shader = MUResourcesManager::GetResourcesManager()->GetProgram("browser");
}

void NBrowserRenderer::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
{
	rect = CefRect(0, 0, Width, Height);
}

void NBrowserRenderer::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, mu_int32 w, mu_int32 h)
{
	const auto immediateContext = MUGraphics::GetImmediateContext();

	if (TextureState == Diligent::RESOURCE_STATE_SHADER_RESOURCE)
	{
		Diligent::StateTransitionDesc barrier(Texture->GetTexture(), Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
		immediateContext->TransitionResourceStates(1u, &barrier);
		TextureState = Diligent::RESOURCE_STATE_COPY_DEST;
	}

	immediateContext->UpdateTexture(
		Texture->GetTexture(),
		0, 0,
		Diligent::Box(0, w, 0, h),
		Diligent::TextureSubResData(buffer, w * 4),
		Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE,
		Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE
	);
}
#endif