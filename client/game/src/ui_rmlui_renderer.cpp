#include "mu_precompiled.h"
#include "ui_rmlui_renderer.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
#include "mu_graphics.h"
#include "mu_config.h"
#include "mu_renderstate.h"
#include "mu_textures.h"
#include "mu_resourcesmanager.h"
#include <MapHelper.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace UIRmlUI
{
	constexpr mu_float IsLinear = static_cast<mu_float>(false);

#pragma pack(4)
	struct RmlAttribs
	{
		glm::mat4 Transform;
		Rml::Vector2f Translation;
		mu_float IsLinear;
	};
#pragma pack()

    NRenderInterface::NRenderInterface() : Projection(glm::mat4(1.0f)), Transform(glm::mat4(1.0f)), Width(0u), Height(0u) {}
	NRenderInterface::~NRenderInterface() {}

	const mu_boolean NRenderInterface::Initialize()
	{
		const auto device = MUGraphics::GetDevice();
		const auto immediateContext = MUGraphics::GetImmediateContext();

		ColorShader = MUResourcesManager::GetProgram("rmlui_color");
		if (ColorShader == NInvalidShader)
		{
			return false;
		}

		TextureShader = MUResourcesManager::GetProgram("rmlui_texture");
		if (TextureShader == NInvalidShader)
		{
			return false;
		}

		// Camera Uniform
		{
			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
			bufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			bufferDesc.Size = sizeof(RmlAttribs);

			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, nullptr, &buffer);
			if (buffer == nullptr)
			{
				return false;
			}

			RmlUniform = buffer;
		}

		return true;
	}

    void NRenderInterface::BeginFrame()
	{
		const auto swapchain = MUGraphics::GetSwapChain();
		auto *pDSV = swapchain->GetDepthBufferDSV();

		auto &swapchainDesc = swapchain->GetDesc();
		Width = static_cast<mu_float>(swapchainDesc.Width);
		Height = static_cast<mu_float>(swapchainDesc.Height);

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
		Transform = glm::mat4(1.0f);
        MURenderState::SetViewTransform(GLMFromFloat4x4(Diligent::float4x4::Identity()), Projection, Projection, MURenderState::GetShadowView(), MURenderState::GetShadowProjection());

		UseScissor = false;
		UseTransform = false;
    }

    void NRenderInterface::EndFrame()
    {
    }

	void NRenderInterface::ReleaseGarbage()
	{
		DynamicBuffersToRelease.clear();
	}

    mu_boolean NRenderInterface::UpdateVertexBuffer()
	{
        if (DynamicVertexBuffer.get() == nullptr || DynamicVertices.size() > DynamicVertexBuffer->Count)
		{
			NDynamicBufferPtr oldBuffer(std::move(DynamicVertexBuffer));

			const mu_uint32 vertexCount = (
				oldBuffer.get() != nullptr
				? oldBuffer->Count * VertexCountMultiplier
				: StartVertexCount
			);

			DynamicBuffersToRelease.push_back(std::move(oldBuffer));

			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
			bufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			bufferDesc.Size = static_cast<mu_uint64>(sizeof(Rml::Vertex) * vertexCount);

			const auto device = MUGraphics::GetDevice();
			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, nullptr, &buffer);
			if (buffer == nullptr)
			{
				return false;
			}

			DynamicVertexBuffer.reset(
				new_nothrow NDynamicBuffer{
					.Offset = static_cast<mu_uint32>(DynamicVertices.size()),
					.Count = vertexCount,
					.Buffer = buffer
				}
			);

			if (DynamicVertices.empty() == false)
			{
				const auto immediateContext = MUGraphics::GetImmediateContext();
				Diligent::MapHelper<Rml::Vertex> mapped(immediateContext, DynamicVertexBuffer->Buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_NO_OVERWRITE);
				mu_memcpy(mapped, DynamicVertices.data(), sizeof(Rml::Vertex) * DynamicVertices.size());
			}
        }
		else
		{
			const mu_uint32 toUpdateSize = static_cast<mu_uint32>(DynamicVertices.size()) - DynamicVertexBuffer->Offset;

			if (toUpdateSize > 0u)
			{
				const auto immediateContext = MUGraphics::GetImmediateContext();
				Diligent::MapHelper<Rml::Vertex> mapped(immediateContext, DynamicVertexBuffer->Buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_NO_OVERWRITE);
				mu_memcpy(mapped, DynamicVertices.data() + DynamicVertexBuffer->Offset, sizeof(Rml::Vertex) * toUpdateSize);
				DynamicVertexBuffer->Offset += toUpdateSize;
			}
		}

        return true;
	}

	mu_boolean NRenderInterface::UpdateIndexBuffer()
	{
		if (DynamicIndexBuffer.get() == nullptr || DynamicIndices.size() > DynamicIndexBuffer->Count)
		{
			NDynamicBufferPtr oldBuffer(std::move(DynamicIndexBuffer));

			const mu_uint32 indexCount = (
				oldBuffer.get() != nullptr
				? oldBuffer->Count * IndexCountMultiplier
				: StartIndexCount
				);

			DynamicBuffersToRelease.push_back(std::move(oldBuffer));

			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
			bufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
			bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			bufferDesc.Size = static_cast<mu_uint64>(sizeof(mu_int32) * indexCount);

			const auto device = MUGraphics::GetDevice();
			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, nullptr, &buffer);
			if (buffer == nullptr)
			{
				return false;
			}

			DynamicIndexBuffer.reset(
				new_nothrow NDynamicBuffer{
					.Offset = static_cast<mu_uint32>(DynamicVertices.size()),
					.Count = indexCount,
					.Buffer = buffer
				}
			);

			if (DynamicIndices.empty() == false)
			{
				const auto immediateContext = MUGraphics::GetImmediateContext();
				Diligent::MapHelper<mu_int32> mapped(immediateContext, DynamicIndexBuffer->Buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_NO_OVERWRITE);
				mu_memcpy(mapped, DynamicIndices.data(), sizeof(mu_int32) * DynamicIndices.size());
			}
		}
		else
		{
			const mu_uint32 toUpdateSize = static_cast<mu_uint32>(DynamicIndices.size()) - DynamicIndexBuffer->Offset;

			if (toUpdateSize)
			{
				const auto immediateContext = MUGraphics::GetImmediateContext();
				Diligent::MapHelper<mu_int32> mapped(immediateContext, DynamicIndexBuffer->Buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_NO_OVERWRITE);
				mu_memcpy(mapped, DynamicVertices.data() + DynamicIndexBuffer->Offset, sizeof(mu_int32) * toUpdateSize);
				DynamicIndexBuffer->Offset += toUpdateSize;
			}
		}

		return true;
	}

	void NRenderInterface::DrawGeometry(
		Diligent::IBuffer *VertexBuffer,
		mu_uint32 verticesCount,
		mu_uint32 verticesOffset,
		Diligent::IBuffer *IndexBuffer,
		mu_uint32 indicesCount,
		mu_uint32 indicesOffset,
		NGraphicsTexture *Texture
	)
	{
		const auto immediateContext = MUGraphics::GetImmediateContext();
		const auto &renderTargetDesc = MUGraphics::GetRenderTargetDesc();

		if (CurrentScissorMode != RequiredScissorMode)
		{
			CurrentScissorMode = RequiredScissorMode;
			switch (RequiredScissorMode)
			{
			case ScissorMode::Normal:
				{
					Diligent::Rect rect;
					rect.left = ScissorRect.x;
					rect.right = ScissorRect.x + ScissorRect.w;
					rect.top = ScissorRect.y;
					rect.bottom = ScissorRect.y + ScissorRect.h;
					immediateContext->SetScissorRects(1, &rect, Width, Height);
				}
				break;

			case ScissorMode::Stencil:
				{
					immediateContext->SetStencilRef(1u);

					const mu_float left = mu_float(ScissorRect.x);
					const mu_float right = mu_float(ScissorRect.x + ScissorRect.w);
					const mu_float top = mu_float(ScissorRect.y);
					const mu_float bottom = mu_float(ScissorRect.y + ScissorRect.h);

					Rml::Vertex vertices[4];
					vertices[0].position = { left, top };
					vertices[1].position = { right, top };
					vertices[2].position = { right, bottom };
					vertices[3].position = { left, bottom };

					mu_int32 indices[6] = { 0, 2, 1, 0, 3, 2 };

					RenderStencil = true;
					RenderGeometry(vertices, mu_countof(vertices), indices, mu_countof(indices), 0u, Rml::Vector2f(0, 0));
					RenderStencil = false;
				}
				break;
			}
		}

		NFixedPipelineState fixedState = {
			.CombinedShader = Texture != nullptr ? TextureShader : ColorShader,
			.RTVFormat = renderTargetDesc.ColorFormat,
			.DSVFormat = renderTargetDesc.DepthStencilFormat,
		};

		NDynamicPipelineState dynamicState = (
			RenderStencil
			? NDynamicPipelineState {
				.CullMode = Diligent::CULL_MODE_NONE,
				.EnableScissors = RequiredScissorMode == ScissorMode::Normal,
				.ColorWrite = false,
				.AlphaWrite = false,
				.DepthWrite = false,
				.DepthFunc = Diligent::COMPARISON_FUNC_ALWAYS,
				.StencilEnable = true,
				.StencilPassOp = Diligent::STENCIL_OP_REPLACE,
				.SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA,
				.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
				.SrcBlendAlpha = Diligent::BLEND_FACTOR_SRC_ALPHA,
				.DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
			}
			: NDynamicPipelineState {
				.CullMode = Diligent::CULL_MODE_NONE,
				.EnableScissors = RequiredScissorMode == ScissorMode::Normal,
				.AlphaWrite = false,
				.DepthWrite = false,
				.StencilEnable = RequiredScissorMode == ScissorMode::Stencil,
				.StencilFunc = Diligent::COMPARISON_FUNC_EQUAL,
				.SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA,
				.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
				.SrcBlendAlpha = Diligent::BLEND_FACTOR_SRC_ALPHA,
				.DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
			}
		);

		auto pipelineState = GetPipelineState(fixedState, dynamicState);
		if (pipelineState->StaticInitialized == false)
		{
			pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "cbRmlAttribs")->Set(RmlUniform);
			pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "cbRmlAttribs")->Set(RmlUniform);
			pipelineState->StaticInitialized = true;
		}

		NResourceId resourceIds[1] = { Texture != nullptr ? Texture->GetId() : NInvalidUInt32 };
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
		immediateContext->SetIndexBuffer(IndexBuffer, 0u, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

		immediateContext->DrawIndexed(
			Diligent::DrawIndexedAttribs(
				indicesCount,
				Diligent::VT_UINT32,
				Diligent::DRAW_FLAGS::DRAW_FLAG_VERIFY_ALL,
				1u,
				indicesOffset,
				verticesOffset,
				0u
			)
		);
	}

    void NRenderInterface::RenderGeometry(
        Rml::Vertex* vertices,
        mu_int32 num_vertices,
        mu_int32* indices,
        mu_int32 num_indices,
        const Rml::TextureHandle texture,
        const Rml::Vector2f& translation
    )
	{
		const mu_uint32 verticesOffset = DynamicVertices.size();
		const mu_uint32 indicesOffset = DynamicIndices.size();

		DynamicVertices.insert(DynamicVertices.end(), vertices, vertices + num_vertices);
		DynamicIndices.insert(DynamicIndices.end(), indices, indices + num_indices);

		UpdateVertexBuffer();
		UpdateIndexBuffer();

		const auto immediateContext = MUGraphics::GetImmediateContext();

		// Update Rml Uniform
		{
			Diligent::MapHelper<RmlAttribs> uniform(immediateContext, RmlUniform, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			*uniform = RmlAttribs {
				.Transform = Projection * Transform,
				.Translation = translation,
				.IsLinear = IsLinear,
			};
		}

		DrawGeometry(
			DynamicVertexBuffer->Buffer,
			static_cast<mu_uint32>(num_vertices),
			verticesOffset,
			DynamicIndexBuffer->Buffer,
			static_cast<mu_uint32>(num_indices),
			indicesOffset,
			reinterpret_cast<NGraphicsTexture*>(texture)
		);
    }

	Rml::CompiledGeometryHandle NRenderInterface::CompileGeometry(
		Rml::Vertex *vertices,
		mu_int32 num_vertices,
		mu_int32 *indices,
		mu_int32 num_indices,
		Rml::TextureHandle texture
	)
    {
		Diligent::RefCntAutoPtr<Diligent::IBuffer> vertexBuffer, indexBuffer;

		// Vertex Buffer
		{
			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_IMMUTABLE;
			bufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
			bufferDesc.Size = static_cast<mu_uint64>(sizeof(Rml::Vertex) * num_vertices);

			Diligent::BufferData bufferData;
			bufferData.pData = vertices;
			bufferData.DataSize = sizeof(Rml::Vertex) * num_vertices;

			const auto device = MUGraphics::GetDevice();
			device->CreateBuffer(bufferDesc, &bufferData, &vertexBuffer);
			if (vertexBuffer == nullptr)
			{
				return (uintptr_t)0u;
			}
		}

		// Index Buffer
		{
			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_IMMUTABLE;
			bufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
			bufferDesc.Size = static_cast<mu_uint64>(sizeof(mu_int32) * num_indices);

			Diligent::BufferData bufferData;
			bufferData.pData = indices;
			bufferData.DataSize = sizeof(mu_int32) * num_indices;

			const auto device = MUGraphics::GetDevice();
			device->CreateBuffer(bufferDesc, &bufferData, &indexBuffer);
			if (indexBuffer == nullptr)
			{
				return (uintptr_t)0u;
			}
		}

		NImmutableBuffer *buffer = new NImmutableBuffer{
			.VertexBuffer = std::make_unique<NDynamicBuffer>(NDynamicBuffer {
				.Offset = static_cast<mu_uint32>(num_vertices),
				.Count = static_cast<mu_uint32>(num_vertices),
				.Buffer = vertexBuffer
			}),
			.IndexBuffer = std::make_unique<NDynamicBuffer>(NDynamicBuffer {
				.Offset = static_cast<mu_uint32>(num_indices),
				.Count = static_cast<mu_uint32>(num_indices),
				.Buffer = indexBuffer
			}),
			.Texture = reinterpret_cast<NGraphicsTexture*>(texture),
		};

		return reinterpret_cast<Rml::CompiledGeometryHandle>(buffer);
	}

	void NRenderInterface::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f &translation)
	{
		NImmutableBuffer *buffer = reinterpret_cast<NImmutableBuffer *>(geometry);
		const auto immediateContext = MUGraphics::GetImmediateContext();

		// Update Rml Uniform
		{
			Diligent::MapHelper<RmlAttribs> uniform(immediateContext, RmlUniform, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			*uniform = RmlAttribs {
				.Transform = Projection * Transform,
				.Translation = translation,
				.IsLinear = IsLinear,
			};
		}

		DrawGeometry(
			buffer->VertexBuffer->Buffer,
			buffer->VertexBuffer->Count,
			0u,
			buffer->IndexBuffer->Buffer,
			buffer->IndexBuffer->Count,
			0u,
			buffer->Texture
		);
	}

	void NRenderInterface::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry)
	{
		NImmutableBuffer *buffer = reinterpret_cast<NImmutableBuffer *>(geometry);
		if (buffer == nullptr) return;
		delete buffer;
	}

    void NRenderInterface::EnableScissorRegion(mu_boolean enable)
    {
		UseScissor = enable;

		if (UseScissor)
		{
			if (UseTransform) CurrentScissorMode = ScissorMode::None;
			else RequiredScissorMode = ScissorMode::Normal;
		}
		else
		{
			RequiredScissorMode = ScissorMode::None;
		}
    }

    void NRenderInterface::SetScissorRegion(mu_int32 x, mu_int32 y, mu_int32 width, mu_int32 height)
    {
        ScissorRect.x = x;
        ScissorRect.y = y;
        ScissorRect.w = width;
        ScissorRect.h = height;
		RequiredScissorMode = UseScissor ? ScissorMode::None : UseTransform ? ScissorMode::Stencil : ScissorMode::Normal;
    }

    mu_boolean NRenderInterface::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source)
    {
		auto texture = MUTextures::Load(source, MUTextures::CalculateSamplerFlags("linear", "clamp"));
		if (!texture)
		{
			return false;
		}

		texture_dimensions.x = static_cast<mu_int32>(texture->GetWidth());
		texture_dimensions.y = static_cast<mu_int32>(texture->GetHeight());
		texture_handle = reinterpret_cast<Rml::TextureHandle>(texture.release());

		return true;
	}

	mu_boolean NRenderInterface::GenerateTexture(Rml::TextureHandle &texture_handle, const Rml::byte *source, const Rml::Vector2i &source_dimensions)
	{
		auto device = MUGraphics::GetDevice();

		const mu_uint32 width = static_cast<mu_uint32>(source_dimensions.x);
		const mu_uint32 height = static_cast<mu_uint32>(source_dimensions.y);
		const mu_uint32 bpp = 32;
		const mu_uint32 bitmapSize = width * height * (bpp / 8);
		const mu_uint8 *bitmapBuffer = source;

		std::vector<Diligent::TextureSubResData> subresources;
		Diligent::TextureSubResData subresource;
		subresource.pData = bitmapBuffer;
		subresource.Stride = width * (bpp / 8);
		subresources.push_back(subresource);

		Diligent::TextureDesc textureDesc;
		textureDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
		textureDesc.Usage = Diligent::USAGE_IMMUTABLE;
		textureDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;

		Diligent::TextureData textureData(subresources.data(), static_cast<mu_uint32>(subresources.size()));
		Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
		device->CreateTexture(textureDesc, &textureData, &texture);
		if (texture == nullptr)
		{
			return false;
		}

		const auto immediateContext = MUGraphics::GetImmediateContext();
		Diligent::StateTransitionDesc barrier(texture, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
		immediateContext->TransitionResourceStates(1u, &barrier);
		texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE)->SetSampler(GetTextureSampler(MUTextures::CalculateSamplerFlags("linear", "clamp"))->Sampler);

		MUGraphics::IncreaseTransactions();
		MUGraphics::CheckIfRequireFlushContext();

		texture_handle = reinterpret_cast<Rml::TextureHandle>(new NGraphicsTexture(MUTextures::GenerateTextureId(), texture, width, height, true));

		return true;
	}

    void NRenderInterface::ReleaseTexture(Rml::TextureHandle texture_handle)
    {
		auto *texture = reinterpret_cast<NGraphicsTexture *>(texture_handle);
		if (texture == nullptr) return;
		delete texture;
    }

	void NRenderInterface::SetTransform(const Rml::Matrix4f *transform)
	{
		if (transform != nullptr)
		{
			UseTransform = false;
			Transform = glm::mat4(1.0f);
		}
		else
		{
			UseTransform = true;
			Transform = glm::make_mat4(transform->data());
		}

		if (UseScissor)
		{
			if (UseTransform) CurrentScissorMode = ScissorMode::None;
			else RequiredScissorMode = ScissorMode::Normal;
		}
	}
}
#endif