#include "mu_precompiled.h"
#include "ui_noesisgui_renderdevice.h"
#include "ui_noesisgui_rendertarget.h"
#include "ui_noesisgui_texture.h"
#include "ui_noesisgui_consts.h"
#include "mu_resourcesmanager.h"
#include "mu_graphics.h"
#include "mu_config.h"
#include <MapHelper.hpp>

namespace UINoesis
{
	Diligent::TEXTURE_FORMAT GetTextureFormat(Noesis::TextureFormat::Enum format, const mu_boolean sRGB)
	{
		switch (format)
		{
		case Noesis::TextureFormat::RGBA8:
		case Noesis::TextureFormat::RGBX8:
			return sRGB ? Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB : Diligent::TEX_FORMAT_RGBA8_UNORM;
		case Noesis::TextureFormat::R8: return Diligent::TEX_FORMAT_R8_UNORM;
		default: NS_ASSERT_UNREACHABLE;
		}
	}

	const mu_uint32 GetTextureBytesPerPixel(const Diligent::TEXTURE_FORMAT format)
	{
		switch (format)
		{
		case Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB: return 4;
		case Diligent::TEX_FORMAT_RGBA8_UNORM: return 4;
		case Diligent::TEX_FORMAT_R8_UNORM: return 1;
		default: NS_ASSERT_UNREACHABLE;
		}
	}

	const mu_uint64 CalculateTextureSize(const Diligent::TEXTURE_FORMAT format, mu_uint32 width, mu_uint32 height, const mu_uint32 levels)
	{
		const auto bytesPerPixel = static_cast<mu_uint64>(GetTextureBytesPerPixel(format));

		mu_uint64 size = 0;
		for (mu_uint32 l = 0; l < levels; ++l)
		{
			size += width * height * bytesPerPixel;
			width = std::max(width >> 1u, 1u);
			height = std::max(height >> 1u, 1u);
		}

		return size;
	}

	uint8_t GetAttributeComponents(uint32_t type)
	{
		switch (type)
		{
		case Noesis::Shader::Vertex::Format::Attr::Type::Float: return 1u;
		case Noesis::Shader::Vertex::Format::Attr::Type::Float2: return 2u;
		case Noesis::Shader::Vertex::Format::Attr::Type::Float4: return 4u;
		case Noesis::Shader::Vertex::Format::Attr::Type::UByte4Norm: return 4u;
		case Noesis::Shader::Vertex::Format::Attr::Type::UShort4Norm: return 4u;
		default: NS_ASSERT_UNREACHABLE;
		}
	}

	Diligent::VALUE_TYPE GetAttributeType(uint32_t type)
	{
		switch (type)
		{
		case Noesis::Shader::Vertex::Format::Attr::Type::Float: return Diligent::VT_FLOAT32;
		case Noesis::Shader::Vertex::Format::Attr::Type::Float2: return Diligent::VT_FLOAT32;
		case Noesis::Shader::Vertex::Format::Attr::Type::Float4: return Diligent::VT_FLOAT32;
		case Noesis::Shader::Vertex::Format::Attr::Type::UByte4Norm: return Diligent::VT_UINT8;
		case Noesis::Shader::Vertex::Format::Attr::Type::UShort4Norm: return Diligent::VT_UINT16;
		default: NS_ASSERT_UNREACHABLE;
		}
	}

	mu_boolean GetAttributeIsNormalized(uint32_t type)
	{
		switch (type)
		{
		case Noesis::Shader::Vertex::Format::Attr::Type::Float: return false;
		case Noesis::Shader::Vertex::Format::Attr::Type::Float2: return false;
		case Noesis::Shader::Vertex::Format::Attr::Type::Float4: return false;
		case Noesis::Shader::Vertex::Format::Attr::Type::UByte4Norm: return true;
		case Noesis::Shader::Vertex::Format::Attr::Type::UShort4Norm: return true;
		default: NS_ASSERT_UNREACHABLE;
		}
	}

	DERenderDevice::DERenderDevice(const mu_boolean sRGB)
	{
		Programs.resize(static_cast<size_t>(Noesis::Shader::Count), NInvalidShader);
		FillCaps(sRGB);
		CreateShaderResources();
		CreateShaders();
		CreateUniforms();
		CreateDummyTexture();
		CreateBuffers();
		InitializeShaderResources();
	}

	DERenderDevice::~DERenderDevice()
	{
		ResourceBindingsManager.Destroy();
	}

	void DERenderDevice::ResetShaders()
	{
		std::fill(Programs.begin(), Programs.end(), NInvalidShader);
	}

	void DERenderDevice::FillCaps(const mu_boolean sRGB)
	{
		const auto device = MUGraphics::GetDevice();
		const auto &deviceInfo = device->GetDeviceInfo();
		const auto deviceType = MUGraphics::GetDeviceType();
		IsCombinedSampler = deviceType == Diligent::RENDER_DEVICE_TYPE_GL || deviceType == Diligent::RENDER_DEVICE_TYPE_GLES;
		Caps.centerPixelOffset = 0.0f;
		Caps.linearRendering = sRGB;
		Caps.subpixelRendering = (
			deviceType == Diligent::RENDER_DEVICE_TYPE_D3D11 ||
			deviceType == Diligent::RENDER_DEVICE_TYPE_D3D12 ||
			deviceType == Diligent::RENDER_DEVICE_TYPE_VULKAN ||
			deviceType == Diligent::RENDER_DEVICE_TYPE_METAL
		);
		Caps.depthRangeZeroToOne = (
			deviceType != Diligent::RENDER_DEVICE_TYPE_GL &&
			deviceType != Diligent::RENDER_DEVICE_TYPE_GLES
		);
		
	}

	void DERenderDevice::CreateShaderResources()
	{
		const auto device = MUGraphics::GetDevice();

		Diligent::PipelineResourceSignatureDesc prsDesc;
#ifndef NDEBUG
		prsDesc.Name = "NoesisGUI";
#endif

		// clang-format off
		const Diligent::PipelineResourceDesc Resources[] =
		{
			Diligent::PipelineResourceDesc(Diligent::SHADER_TYPE_VERTEX | Diligent::SHADER_TYPE_PIXEL, "ProjectionBuffer", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC),
			Diligent::PipelineResourceDesc(Diligent::SHADER_TYPE_PIXEL, "Buffer0", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC),
			Diligent::PipelineResourceDesc(Diligent::SHADER_TYPE_VERTEX, "Buffer1", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC),
			Diligent::PipelineResourceDesc(Diligent::SHADER_TYPE_PIXEL, "Buffer2", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC),
			Diligent::PipelineResourceDesc(Diligent::SHADER_TYPE_PIXEL, "g_Pattern", 1, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE),
			Diligent::PipelineResourceDesc(Diligent::SHADER_TYPE_PIXEL, "g_Ramps", 1, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE),
			Diligent::PipelineResourceDesc(Diligent::SHADER_TYPE_PIXEL, "g_Image", 1, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE),
			Diligent::PipelineResourceDesc(Diligent::SHADER_TYPE_PIXEL, "g_Glyphs", 1, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE),
			Diligent::PipelineResourceDesc(Diligent::SHADER_TYPE_PIXEL, "g_Shadow", 1, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE),
			Diligent::PipelineResourceDesc(Diligent::SHADER_TYPE_PIXEL, "g_Pattern_sampler", 1, Diligent::SHADER_RESOURCE_TYPE_SAMPLER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE),
			Diligent::PipelineResourceDesc(Diligent::SHADER_TYPE_PIXEL, "g_Ramps_sampler", 1, Diligent::SHADER_RESOURCE_TYPE_SAMPLER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE),
			Diligent::PipelineResourceDesc(Diligent::SHADER_TYPE_PIXEL, "g_Image_sampler", 1, Diligent::SHADER_RESOURCE_TYPE_SAMPLER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE),
			Diligent::PipelineResourceDesc(Diligent::SHADER_TYPE_PIXEL, "g_Glyphs_sampler", 1, Diligent::SHADER_RESOURCE_TYPE_SAMPLER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE),
			Diligent::PipelineResourceDesc(Diligent::SHADER_TYPE_PIXEL, "g_Shadow_sampler", 1, Diligent::SHADER_RESOURCE_TYPE_SAMPLER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE),
		};
		// clang-format on

		prsDesc.BindingIndex = 0;
		prsDesc.Resources = Resources;
		prsDesc.NumResources = mu_countof(Resources);

		device->CreatePipelineResourceSignature(prsDesc, &ResourceSignature);
		VERIFY_EXPR(ResourceSignature);
	}

	void DERenderDevice::InitializeShaderResources()
	{
		ResourceSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "ProjectionBuffer")->Set(VertexUniforms[0]);
		ResourceSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Buffer1")->Set(VertexUniforms[1]);
		ResourceSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Buffer0")->Set(FragmentUniforms[0]);
		ResourceSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Buffer2")->Set(FragmentUniforms[1]);
	}

	void DERenderDevice::CreateShaders()
	{
		for (mu_uint32 n = 0; n < Noesis::Shader::Vertex::Format::Count; ++n)
		{
			auto &layout = InputLayouts[n];
			layout.Clear();

			uint32_t attributes = Noesis::AttributesForFormat[n];
			for (uint32_t j = 0, index = 0; j < Noesis::Shader::Vertex::Format::Attr::Count; j++)
			{
				if ((attributes & (1 << j)) == 0) continue;
				uint8_t t = Noesis::TypeForAttr[j];
				layout.Add(
					Diligent::LayoutElement(
						j,
						0,
						GetAttributeComponents(t),
						GetAttributeType(t),
						GetAttributeIsNormalized(t),
						Diligent::INPUT_ELEMENT_FREQUENCY_PER_VERTEX
					)
				);
			}
		}

		EnsureProgram(Noesis::Shader::RGBA);
		EnsureProgram(Noesis::Shader::Mask);
		EnsureProgram(Noesis::Shader::Clear);

		EnsureProgram(Noesis::Shader::Path_Solid);
		EnsureProgram(Noesis::Shader::Path_Linear);
		EnsureProgram(Noesis::Shader::Path_Radial);
		EnsureProgram(Noesis::Shader::Path_Pattern);

		EnsureProgram(Noesis::Shader::Path_AA_Solid);
		EnsureProgram(Noesis::Shader::Path_AA_Linear);
		EnsureProgram(Noesis::Shader::Path_AA_Radial);
		EnsureProgram(Noesis::Shader::Path_AA_Pattern);

		EnsureProgram(Noesis::Shader::SDF_Solid);
		EnsureProgram(Noesis::Shader::SDF_Linear);
		EnsureProgram(Noesis::Shader::SDF_Radial);
		EnsureProgram(Noesis::Shader::SDF_Pattern);

		EnsureProgram(Noesis::Shader::Opacity_Solid);
		EnsureProgram(Noesis::Shader::Opacity_Linear);
		EnsureProgram(Noesis::Shader::Opacity_Radial);
		EnsureProgram(Noesis::Shader::Opacity_Pattern);
	}

	void DERenderDevice::CreateUniforms()
	{
		const auto device = MUGraphics::GetDevice();

		// Vertex Uniform 1
		{
			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
			bufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			bufferDesc.Size = sizeof(glm::mat4) * 2;

			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, nullptr, &buffer);
			VertexUniforms[0] = buffer;
		}

		// Vertex Uniform 2
		{
			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
			bufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			bufferDesc.Size = sizeof(glm::vec4) * 1;

			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, nullptr, &buffer);
			VertexUniforms[1] = buffer;
		}

		// Pixel Uniform 1
		{
			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
			bufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			bufferDesc.Size = sizeof(glm::vec4) * 3;

			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, nullptr, &buffer);
			FragmentUniforms[0] = buffer;
		}

		// Pixel Uniform 2
		{
			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
			bufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			bufferDesc.Size = sizeof(glm::vec4) * 32;

			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, nullptr, &buffer);
			FragmentUniforms[1] = buffer;
		}

		/*const auto immediateContext = MUGraphics::GetImmediateContext();
		Diligent::StateTransitionDesc barriers[] = {
			Diligent::StateTransitionDesc(VertexUniforms[0], Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE),
			Diligent::StateTransitionDesc(VertexUniforms[1], Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE),
			Diligent::StateTransitionDesc(FragmentUniforms[0], Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE),
			Diligent::StateTransitionDesc(FragmentUniforms[1], Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE),
		};
		immediateContext->TransitionResourceStates(mu_countof(barriers), barriers);*/
	}

	/*
		Create a dummy texture so we always attach a resource to the fragment shader to avoid false validation issues.
	*/
	void DERenderDevice::CreateDummyTexture()
	{
		const auto device = MUGraphics::GetDevice();
		const auto immediateContext = MUGraphics::GetImmediateContext();
		mu_uint32 dummyPixel = 0xFFFFFFFF;

		std::vector<Diligent::TextureSubResData> subresources;
		Diligent::TextureSubResData subresource;
		subresource.pData = &dummyPixel;
		subresource.Stride = sizeof(dummyPixel);
		subresources.push_back(subresource);

		Diligent::TextureDesc textureDesc;
#if NEXTMU_COMPILE_DEBUG == 1
		textureDesc.Name = "NGUI Dummy Texture";
#endif
		textureDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
		textureDesc.Width = 1;
		textureDesc.Height = 1;
		textureDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
		textureDesc.Usage = Diligent::USAGE_IMMUTABLE;
		textureDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;

		Diligent::TextureData textureData(subresources.data(), static_cast<mu_uint32>(subresources.size()));
		Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
		device->CreateTexture(textureDesc, &textureData, &texture);
		if (texture == nullptr)
		{
			return;
		}

		Diligent::StateTransitionDesc barrier(texture, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
		immediateContext->TransitionResourceStates(1, &barrier);

		const auto deviceType = MUGraphics::GetDeviceType();
		const mu_boolean isGL = deviceType == Diligent::RENDER_DEVICE_TYPE_GL || deviceType == Diligent::RENDER_DEVICE_TYPE_GLES;
		DummyTexture = Noesis::MakePtr<DETexture>(
			1,
			1,
			1,
			1,
			isGL,
			true,
			Diligent::TEX_FORMAT_RGBA8_UNORM,
			texture
		);
	}

	void DERenderDevice::CreateBuffers()
	{
		const auto device = MUGraphics::GetDevice();

		// Vertex Buffer
		{
			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_DEFAULT;
			bufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
			bufferDesc.Size = DYNAMIC_VB_SIZE;

			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, nullptr, &buffer);
			GPUVertexBuffer = buffer;
		}

		// Index Buffer
		{
			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_DEFAULT;
			bufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
			bufferDesc.Size = DYNAMIC_VB_SIZE;

			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, nullptr, &buffer);
			GPUIndexBuffer = buffer;
		}
	}

	const mu_char *ShaderIDs[Noesis::Shader::Count] = {
		// Shader for debug modes
		"ng_rgba",

		// Stencil-only rendering for masks
		"ng_mask",

		// Shader used for clearing render target
		"ng_clear",

		// Shaders for rendering geometry paths without PPAA
		"ng_path_solid",
		"ng_path_linear",
		"ng_path_radial",
		"ng_path_pattern",
		"ng_path_pattern_clamp",
		"ng_path_pattern_repeat",
		"ng_path_pattern_mirroru",
		"ng_path_pattern_mirrorv",
		"ng_path_pattern_mirror",

		// Shaders for rendering geometry paths with PPAA
		"ng_path_aa_solid",
		"ng_path_aa_linear",
		"ng_path_aa_radial",
		"ng_path_aa_pattern",
		"ng_path_aa_pattern_clamp",
		"ng_path_aa_pattern_repeat",
		"ng_path_aa_pattern_mirroru",
		"ng_path_aa_pattern_mirrorv",
		"ng_path_aa_pattern_mirror",

		// Shaders for rendering distance fields
		"ng_sdf_solid",
		"ng_sdf_linear",
		"ng_sdf_radial",
		"ng_sdf_pattern",
		"ng_sdf_pattern_clamp",
		"ng_sdf_pattern_repeat",
		"ng_sdf_pattern_mirroru",
		"ng_sdf_pattern_mirrorv",
		"ng_sdf_pattern_mirror",

		// Shaders for rendering distance fields with subpixel rendering.
		// These shaders are only used when the device reports support for
		// subpixel rendering in DeviceCaps. Otherwise SDF_* shaders are used
		"ng_sdf_lcd_solid",
		"ng_sdf_lcd_linear",
		"ng_sdf_lcd_radial",
		"ng_sdf_lcd_pattern",
		"ng_sdf_lcd_pattern_clamp",
		"ng_sdf_lcd_pattern_repeat",
		"ng_sdf_lcd_pattern_mirroru",
		"ng_sdf_lcd_pattern_mirrorv",
		"ng_sdf_lcd_pattern_mirror",

		// Shaders for offscreen rendering
		"ng_opacity_solid",
		"ng_opacity_linear",
		"ng_opacity_radial",
		"ng_opacity_pattern",
		"ng_opacity_pattern_clamp",
		"ng_opacity_pattern_repeat",
		"ng_opacity_pattern_mirroru",
		"ng_opacity_pattern_mirrorv",
		"ng_opacity_pattern_mirror",

		"ng_upsample",
		"ng_downsample",

		"ng_shadow",
		"ng_blur",
		"ng_custom_effect",
	};

	const mu_char *ShaderPaths[Noesis::Shader::Count] = {
		// Shader for debug modes
		"rgba",

		// Stencil-only rendering for masks
		"mask",

		// Shader used for clearing render target
		"clear",

		// Shaders for rendering geometry paths without PPAA
		"path_solid",
		"path_linear",
		"path_radial",
		"path_pattern",
		"path_pattern_clamp",
		"path_pattern_repeat",
		"path_pattern_mirroru",
		"path_pattern_mirrorv",
		"path_pattern_mirror",

		// Shaders for rendering geometry paths with PPAA
		"path_aa_solid",
		"path_aa_linear",
		"path_aa_radial",
		"path_aa_pattern",
		"path_aa_pattern_clamp",
		"path_aa_pattern_repeat",
		"path_aa_pattern_mirroru",
		"path_aa_pattern_mirrorv",
		"path_aa_pattern_mirror",

		// Shaders for rendering distance fields
		"sdf_solid",
		"sdf_linear",
		"sdf_radial",
		"sdf_pattern",
		"sdf_pattern_clamp",
		"sdf_pattern_repeat",
		"sdf_pattern_mirroru",
		"sdf_pattern_mirrorv",
		"sdf_pattern_mirror",

		// Shaders for rendering distance fields with subpixel rendering.
		// These shaders are only used when the device reports support for
		// subpixel rendering in DeviceCaps. Otherwise SDF_* shaders are used
		"sdf_lcd_solid",
		"sdf_lcd_linear",
		"sdf_lcd_radial",
		"sdf_lcd_pattern",
		"sdf_lcd_pattern_clamp",
		"sdf_lcd_pattern_repeat",
		"sdf_lcd_pattern_mirroru",
		"sdf_lcd_pattern_mirrorv",
		"sdf_lcd_pattern_mirror",

		// Shaders for offscreen rendering
		"opacity_solid",
		"opacity_linear",
		"opacity_radial",
		"opacity_pattern",
		"opacity_pattern_clamp",
		"opacity_pattern_repeat",
		"opacity_pattern_mirroru",
		"opacity_pattern_mirrorv",
		"opacity_pattern_mirror",

		"upsample",
		"downsample",

		"shadow",
		"blur",
		"custom_effect",
	};

	void GetVertexMacros(const Noesis::Shader::Vertex::Enum shader, Diligent::ShaderMacroHelper &macros)
	{
		using namespace Noesis;

#define VSHADER_1(x0) macros.AddShaderMacro(#x0, 1);
#define VSHADER_2(x0, x1) macros.AddShaderMacro(#x0, 1);macros.AddShaderMacro(#x1, 1);
#define VSHADER_3(x0, x1, x2) macros.AddShaderMacro(#x0, 1);macros.AddShaderMacro(#x1, 1);macros.AddShaderMacro(#x2, 1);
#define VSHADER_4(x0, x1, x2, x3) macros.AddShaderMacro(#x0, 1);macros.AddShaderMacro(#x1, 1);macros.AddShaderMacro(#x2, 1);macros.AddShaderMacro(#x3, 1);
#define VSHADER_5(x0, x1, x2, x3, x4) macros.AddShaderMacro(#x0, 1);macros.AddShaderMacro(#x1, 1);macros.AddShaderMacro(#x2, 1);macros.AddShaderMacro(#x3, 1);macros.AddShaderMacro(#x4, 1);

		switch (shader)
		{
		case Shader::Vertex::Pos: break;
		case Shader::Vertex::PosColor: VSHADER_1(HAS_COLOR); break;
		case Shader::Vertex::PosTex0: VSHADER_1(HAS_UV0); break;
		case Shader::Vertex::PosTex0Rect: VSHADER_2(HAS_UV0, HAS_RECT); break;
		case Shader::Vertex::PosTex0RectTile: VSHADER_3(HAS_UV0, HAS_RECT, HAS_TILE); break;
		case Shader::Vertex::PosColorCoverage: VSHADER_2(HAS_COLOR, HAS_COVERAGE); break;
		case Shader::Vertex::PosTex0Coverage: VSHADER_2(HAS_UV0, HAS_COVERAGE); break;
		case Shader::Vertex::PosTex0CoverageRect: VSHADER_3(HAS_UV0, HAS_COVERAGE, HAS_RECT); break;
		case Shader::Vertex::PosTex0CoverageRectTile: VSHADER_4(HAS_UV0, HAS_COVERAGE, HAS_RECT, HAS_TILE); break;
		case Shader::Vertex::PosColorTex1_SDF: VSHADER_3(HAS_COLOR, HAS_UV1, SDF); break;
		case Shader::Vertex::PosTex0Tex1_SDF: VSHADER_3(HAS_UV0, HAS_UV1, SDF); break;
		case Shader::Vertex::PosTex0Tex1Rect_SDF: VSHADER_4(HAS_UV0, HAS_UV1, HAS_RECT, SDF); break;
		case Shader::Vertex::PosTex0Tex1RectTile_SDF: VSHADER_5(HAS_UV0, HAS_UV1, HAS_RECT, HAS_TILE, SDF); break;
		case Shader::Vertex::PosColorTex1: VSHADER_2(HAS_COLOR, HAS_UV1); break;
		case Shader::Vertex::PosTex0Tex1: VSHADER_2(HAS_UV0, HAS_UV1); break;
		case Shader::Vertex::PosTex0Tex1Rect: VSHADER_3(HAS_UV0, HAS_UV1, HAS_RECT); break;
		case Shader::Vertex::PosTex0Tex1RectTile: VSHADER_4(HAS_UV0, HAS_UV1, HAS_RECT, HAS_TILE); break;
		case Shader::Vertex::PosColorTex0Tex1: VSHADER_3(HAS_COLOR, HAS_UV0, HAS_UV1); break;
		case Shader::Vertex::PosTex0Tex1_Downsample: VSHADER_3(HAS_UV0, HAS_UV1, DOWNSAMPLE); break;
		case Shader::Vertex::PosColorTex1Rect: VSHADER_3(HAS_COLOR, HAS_UV1, HAS_RECT); break;
		case Shader::Vertex::PosColorTex0RectImagePos: VSHADER_4(HAS_COLOR, HAS_UV0, HAS_RECT, HAS_IMAGE_POSITION); break;
		default: NS_ASSERT_UNREACHABLE;
		}
	}

	void GetPixelMacros(const Noesis::Shader::Enum shader, Diligent::ShaderMacroHelper &macros)
	{
		using namespace Noesis;

#define FSHADER(x) macros.AddShaderMacro("EFFECT_"#x, 1);
#define FSHADER2(x, y) macros.AddShaderMacro("EFFECT_"#x, 1); macros.AddShaderMacro("PAINT_"#y, 1);
#define FSHADER3(x, y, z) macros.AddShaderMacro("EFFECT_"#x, 1); macros.AddShaderMacro("PAINT_"#y, 1); macros.AddShaderMacro(#z"_PATTERN", 1);

		switch (shader)
		{
		case Shader::RGBA: FSHADER(RGBA); break;
		case Shader::Mask: FSHADER(MASK); break;
		case Shader::Clear: FSHADER(CLEAR); break;

		case Shader::Path_Solid: FSHADER2(PATH, SOLID); break;
		case Shader::Path_Linear: FSHADER2(PATH, LINEAR); break;
		case Shader::Path_Radial: FSHADER2(PATH, RADIAL); break;
		case Shader::Path_Pattern: FSHADER2(PATH, PATTERN); break;
		case Shader::Path_Pattern_Clamp: FSHADER3(PATH, PATTERN, CLAMP); break;
		case Shader::Path_Pattern_Repeat: FSHADER3(PATH, PATTERN, REPEAT); break;
		case Shader::Path_Pattern_MirrorU: FSHADER3(PATH, PATTERN, MIRRORU); break;
		case Shader::Path_Pattern_MirrorV: FSHADER3(PATH, PATTERN, MIRRORV); break;
		case Shader::Path_Pattern_Mirror: FSHADER3(PATH, PATTERN, MIRROR); break;

		case Shader::Path_AA_Solid: FSHADER2(PATH_AA, SOLID); break;
		case Shader::Path_AA_Linear: FSHADER2(PATH_AA, LINEAR); break;
		case Shader::Path_AA_Radial: FSHADER2(PATH_AA, RADIAL); break;
		case Shader::Path_AA_Pattern: FSHADER2(PATH_AA, PATTERN); break;
		case Shader::Path_AA_Pattern_Clamp: FSHADER3(PATH_AA, PATTERN, CLAMP); break;
		case Shader::Path_AA_Pattern_Repeat: FSHADER3(PATH_AA, PATTERN, REPEAT); break;
		case Shader::Path_AA_Pattern_MirrorU: FSHADER3(PATH_AA, PATTERN, MIRRORU); break;
		case Shader::Path_AA_Pattern_MirrorV: FSHADER3(PATH_AA, PATTERN, MIRRORV); break;
		case Shader::Path_AA_Pattern_Mirror: FSHADER3(PATH_AA, PATTERN, MIRROR); break;

		case Shader::SDF_Solid: FSHADER2(SDF, SOLID); break;
		case Shader::SDF_Linear: FSHADER2(SDF, LINEAR); break;
		case Shader::SDF_Radial: FSHADER2(SDF, RADIAL); break;
		case Shader::SDF_Pattern: FSHADER2(SDF, PATTERN); break;
		case Shader::SDF_Pattern_Clamp: FSHADER3(SDF, PATTERN, CLAMP); break;
		case Shader::SDF_Pattern_Repeat: FSHADER3(SDF, PATTERN, REPEAT); break;
		case Shader::SDF_Pattern_MirrorU: FSHADER3(SDF, PATTERN, MIRRORU); break;
		case Shader::SDF_Pattern_MirrorV: FSHADER3(SDF, PATTERN, MIRRORV); break;
		case Shader::SDF_Pattern_Mirror: FSHADER3(SDF, PATTERN, MIRROR); break;

		case Shader::SDF_LCD_Solid: FSHADER2(SDF_LCD, SOLID); break;
		case Shader::SDF_LCD_Linear: FSHADER2(SDF_LCD, LINEAR); break;
		case Shader::SDF_LCD_Radial: FSHADER2(SDF_LCD, RADIAL); break;
		case Shader::SDF_LCD_Pattern: FSHADER2(SDF_LCD, PATTERN); break;
		case Shader::SDF_LCD_Pattern_Clamp: FSHADER3(SDF_LCD, PATTERN, CLAMP); break;
		case Shader::SDF_LCD_Pattern_Repeat: FSHADER3(SDF_LCD, PATTERN, REPEAT); break;
		case Shader::SDF_LCD_Pattern_MirrorU: FSHADER3(SDF_LCD, PATTERN, MIRRORU); break;
		case Shader::SDF_LCD_Pattern_MirrorV: FSHADER3(SDF_LCD, PATTERN, MIRRORV); break;
		case Shader::SDF_LCD_Pattern_Mirror: FSHADER3(SDF_LCD, PATTERN, MIRROR); break;

		case Shader::Opacity_Solid: FSHADER2(OPACITY, SOLID); break;
		case Shader::Opacity_Linear: FSHADER2(OPACITY, LINEAR); break;
		case Shader::Opacity_Radial: FSHADER2(OPACITY, RADIAL); break;
		case Shader::Opacity_Pattern: FSHADER2(OPACITY, PATTERN); break;
		case Shader::Opacity_Pattern_Clamp: FSHADER3(OPACITY, PATTERN, CLAMP); break;
		case Shader::Opacity_Pattern_Repeat: FSHADER3(OPACITY, PATTERN, REPEAT); break;
		case Shader::Opacity_Pattern_MirrorU: FSHADER3(OPACITY, PATTERN, MIRRORU); break;
		case Shader::Opacity_Pattern_MirrorV: FSHADER3(OPACITY, PATTERN, MIRRORV); break;
		case Shader::Opacity_Pattern_Mirror: FSHADER3(OPACITY, PATTERN, MIRROR); break;

		case Shader::Upsample: FSHADER(UPSAMPLE); break;
		case Shader::Downsample: FSHADER(DOWNSAMPLE); break;

		case Shader::Shadow: FSHADER2(SHADOW, SOLID); break;
		case Shader::Blur: FSHADER2(BLUR, SOLID); break;

		default: NS_ASSERT_UNREACHABLE;
		}
	}

	const mu_utf8string GetVertexInputOutput(const Diligent::ShaderMacroArray macros)
	{
		mu_utf8string value;

		value += "struct VSInput\n";
		value += "{\n";
		value += "float2 position: ATTRIB0;\n";
		for (mu_uint32 n = 0; n < macros.Count; ++n)
		{
			const auto &macro = macros.Elements[n];
			if (std::strcmp(macro.Name, "HAS_COLOR") == 0)
			{
				value += "half4 color: ATTRIB1;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_UV0") == 0)
			{
				value += "float2 uv0: ATTRIB2;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_UV1") == 0)
			{
				value += "float2 uv1: ATTRIB3;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_COVERAGE") == 0)
			{
				value += "half coverage: ATTRIB4;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_RECT") == 0)
			{
				value += "float4 rect: ATTRIB5;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_TILE") == 0)
			{
				value += "float4 tile: ATTRIB6;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_IMAGE_POSITION") == 0)
			{
				value += "float4 imagePos: ATTRIB7;\n";
			}
			else if (std::strcmp(macro.Name, "STEREO_RENDERING") == 0)
			{
				value += "uint eyeIndex : SV_InstanceID;\n";
			}
		}
		value += "};\n";

		value += "struct PSInput\n";
		value += "{\n";
		value += "float4 position: SV_POSITION;\n";
		for (mu_uint32 n = 0; n < macros.Count; ++n)
		{
			const auto &macro = macros.Elements[n];
			if (std::strcmp(macro.Name, "HAS_COLOR") == 0)
			{
				value += "nointerpolation half4 color: COLOR0;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_UV0") == 0)
			{
				value += "float2 uv0: TEXCOORD0;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_UV1") == 0)
			{
				value += "float2 uv1: TEXCOORD1;\n";
			}
			else if (std::strcmp(macro.Name, "DOWNSAMPLE") == 0)
			{
				value += "float2 uv2: TEXCOORD2;\n";
				value += "float2 uv3: TEXCOORD3;\n";
			}
			else if (std::strcmp(macro.Name, "SDF") == 0)
			{
				value += "float4 st1: TEXCOORD2;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_COVERAGE") == 0)
			{
				value += "half coverage: COVERAGE;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_RECT") == 0)
			{
				value += "nointerpolation float4 rect: RECT;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_TILE") == 0)
			{
				value += "nointerpolation float4 tile: TILE;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_IMAGE_POSITION") == 0)
			{
				value += "float4 imagePos: IMAGE_POSITION;\n";
			}
			else if (std::strcmp(macro.Name, "STEREO_RENDERING") == 0)
			{
				value += "uint renderTargetIndex: SV_RenderTargetArrayIndex;\n";
			}
		}
		value += "};\n";

		return value;
	}

	const mu_utf8string GetPixelInput(const Diligent::ShaderMacroArray vertexMacros, const Diligent::ShaderMacroArray pixelMacros)
	{
		mu_utf8string value;

		value += "struct PSInput\n";
		value += "{\n";
		value += "float4 position: SV_POSITION;\n";
		for (mu_uint32 n = 0; n < vertexMacros.Count; ++n)
		{
			const auto &macro = vertexMacros.Elements[n];
			if (std::strcmp(macro.Name, "HAS_COLOR") == 0)
			{
				value += "nointerpolation half4 color: COLOR0;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_UV0") == 0)
			{
				value += "float2 uv0: TEXCOORD0;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_UV1") == 0)
			{
				value += "float2 uv1: TEXCOORD1;\n";
			}
			else if (std::strcmp(macro.Name, "DOWNSAMPLE") == 0)
			{
				value += "float2 uv2: TEXCOORD2;\n";
				value += "float2 uv3: TEXCOORD3;\n";
			}
			else if (std::strcmp(macro.Name, "SDF") == 0)
			{
				value += "float4 st1: TEXCOORD2;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_COVERAGE") == 0)
			{
				value += "half coverage: COVERAGE;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_RECT") == 0)
			{
				value += "nointerpolation float4 rect: RECT;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_TILE") == 0)
			{
				value += "nointerpolation float4 tile: TILE;\n";
			}
			else if (std::strcmp(macro.Name, "HAS_IMAGE_POSITION") == 0)
			{
				value += "float4 imagePos: IMAGE_POSITION;\n";
			}
		}
		value += "};\n";

		value += "struct PSOutput\n";
		value += "{\n";
		value += "half4 color: SV_TARGET0;\n";
		for (mu_uint32 n = 0; n < pixelMacros.Count; ++n)
		{
			const auto &macro = pixelMacros.Elements[n];
			if (std::strcmp(macro.Name, "EFFECT_SDF_LCD") == 0)
			{
				value += "half4 alpha: SV_TARGET1;\n";
			}
		}
		value += "};\n";

		return value;
	}

	const mu_shader DERenderDevice::LoadShader(const Noesis::Shader::Enum shader)
	{
		const mu_utf8string id = ShaderIDs[shader];
		auto *manager = MUResourcesManager::GetResourcesManager();
		const mu_shader program = manager->GetProgram(id);
		if (program != NInvalidShader)
		{
			return program;
		}

		auto vertexShader = Noesis::VertexForShader[shader];
		auto format = Noesis::FormatForVertex[vertexShader];
		const auto &layout = InputLayouts[format];

		NShaderSettings settings;
		GetVertexMacros(static_cast<Noesis::Shader::Vertex::Enum>(Noesis::VertexForShader[shader]), settings.VertexMacros);
		GetPixelMacros(shader, settings.PixelMacros);
		settings.InputLayout = layout;
		settings.AppendVertex = GetVertexInputOutput(settings.VertexMacros);
		settings.AppendPixel = GetPixelInput(settings.VertexMacros, settings.PixelMacros);
		settings.ResourceSignatures.push_back(ResourceSignature.RawPtr());

		const mu_utf8string vertexPath = "./data/shaders/noesisgui/shader.vs";
		const mu_utf8string pixelPath = "./data/shaders/noesisgui/shader.fs";
		if (manager->LoadProgram(id, vertexPath, pixelPath, settings) == false)
		{
			return NInvalidShader;
		}

		return manager->GetProgram(id);
	}

	void DERenderDevice::EnsureProgram(const Noesis::Shader::Enum shader)
	{
		if (Programs[shader] != NInvalidShader) return;
		Programs[shader] = LoadShader(shader);
	}

	/// Retrieves device render capabilities
	const Noesis::DeviceCaps &DERenderDevice::GetCaps() const
	{
		return Caps;
	}

	Noesis::Ptr<Noesis::RenderTarget> DERenderDevice::CreateRenderTarget(
		const char *label,
		uint32_t width, uint32_t height,
		uint32_t sampleCount, bool needsStencil,
		Diligent::RefCntAutoPtr<Diligent::ITexture> colorMSAA,
		Diligent::RefCntAutoPtr<Diligent::ITexture> stencil
	)
	{
		const auto device = MUGraphics::GetDevice();
		const mu_boolean sRGB = Caps.linearRendering;

		Diligent::RefCntAutoPtr<Diligent::ITexture> colorTexture;
		Diligent::RefCntAutoPtr<Diligent::ITexture> colorMSAATexture = colorMSAA;
		Diligent::RefCntAutoPtr<Diligent::ITexture> stencilTexture = stencil;

		const auto colorFormat = sRGB ? Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB : Diligent::TEX_FORMAT_RGBA8_UNORM;
		const mu_utf8string textureLabel = label;

		// Create Color Texture
		{
			Diligent::TextureDesc textureDesc;
#if NEXTMU_COMPILE_DEBUG == 1
			textureDesc.Name = textureLabel.c_str();
#endif
			textureDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
			textureDesc.Width = width;
			textureDesc.Height = height;
			textureDesc.Format = colorFormat;
			textureDesc.Usage = Diligent::USAGE_DEFAULT;
			textureDesc.BindFlags = Diligent::BIND_RENDER_TARGET | Diligent::BIND_SHADER_RESOURCE;
			textureDesc.SampleCount = 1;

			device->CreateTexture(textureDesc, nullptr, &colorTexture);
			if (colorTexture == nullptr)
			{
				return nullptr;
			}
		}

		// Create Color MSAA Texture
		if (colorMSAATexture == nullptr && sampleCount > 1)
		{
			Diligent::TextureDesc textureDesc;
#if NEXTMU_COMPILE_DEBUG == 1
			auto textureLabelMSAA = textureLabel + "(MSAA)";
			textureDesc.Name = textureLabelMSAA.c_str();
#endif
			textureDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
			textureDesc.Width = width;
			textureDesc.Height = height;
			textureDesc.Format = colorFormat;
			textureDesc.Usage = Diligent::USAGE_DEFAULT;
			textureDesc.BindFlags = Diligent::BIND_RENDER_TARGET | Diligent::BIND_SHADER_RESOURCE;
			textureDesc.SampleCount = sampleCount;

			device->CreateTexture(textureDesc, nullptr, &colorMSAATexture);
			if (colorMSAATexture == nullptr)
			{
				return nullptr;
			}
		}

		if (stencilTexture == nullptr && needsStencil)
		{
			Diligent::TextureDesc textureDesc;
#if NEXTMU_COMPILE_DEBUG == 1
			auto textureLabelDS = textureLabel + "(DS)";
			textureDesc.Name = textureLabelDS.c_str();
#endif
			textureDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
			textureDesc.Width = width;
			textureDesc.Height = height;
			textureDesc.Format = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
			textureDesc.Usage = Diligent::USAGE_DEFAULT;
			textureDesc.BindFlags = Diligent::BIND_DEPTH_STENCIL;
			textureDesc.SampleCount = sampleCount;

			device->CreateTexture(textureDesc, nullptr, &stencilTexture);
			if (stencilTexture == nullptr)
			{
				return nullptr;
			}
		}

		const auto deviceType = MUGraphics::GetDeviceType();
		const mu_boolean isGL = deviceType == Diligent::RENDER_DEVICE_TYPE_GL || deviceType == Diligent::RENDER_DEVICE_TYPE_GLES;
		return Noesis::MakePtr<DERenderTarget>(
			Noesis::MakePtr<DETexture>(
				width,
				height,
				1,
				sampleCount,
				isGL,
				true,
				colorFormat,
				colorTexture
			),
			colorMSAATexture,
			stencilTexture
		);
	}

	/// Creates render target surface with given dimensions, samples and optional stencil buffer
	Noesis::Ptr<Noesis::RenderTarget> DERenderDevice::CreateRenderTarget(const char *label, uint32_t width, uint32_t height,
														 uint32_t sampleCount, bool needsStencil)
	{
		return CreateRenderTarget(label, width, height, sampleCount, needsStencil, Diligent::RefCntAutoPtr<Diligent::ITexture>(), Diligent::RefCntAutoPtr<Diligent::ITexture>());
	}

	/// Creates render target sharing transient (stencil, colorAA) buffers with the given surface
	Noesis::Ptr<Noesis::RenderTarget> DERenderDevice::CloneRenderTarget(const char *label, Noesis::RenderTarget *surface)
	{
		DERenderTarget *renderTarget = static_cast<DERenderTarget *>(surface);
		DETexture *texture = static_cast<DETexture *>(renderTarget->GetTexture());
		return CreateRenderTarget(
			label,
			texture->GetWidth(),
			texture->GetHeight(),
			texture->GetSampleCount(),
			renderTarget->HasDepthStencil(),
			renderTarget->ColorMSAATexture,
			renderTarget->DepthStencilTexture
		);
	}

	/// Creates texture with given dimensions and format. For immutable textures, the content of
	/// each mipmap is given in 'data'. The passed data is tightly packed (no extra pitch). When
	/// 'data' is null the texture is considered dynamic and will be updated using UpdateTexture()
	Noesis::Ptr<Noesis::Texture> DERenderDevice::CreateTexture(const char *label, uint32_t width, uint32_t height,
											   uint32_t numLevels, Noesis::TextureFormat::Enum format, const void **data)
	{
		const auto device = MUGraphics::GetDevice();
		const auto textureFormat = GetTextureFormat(format, Caps.linearRendering);
		const auto memorySize = CalculateTextureSize(textureFormat, width, height, numLevels);

		std::vector<Diligent::TextureSubResData> subresources;
		if (data != nullptr)
		{
			const auto bytesPerPixel = GetTextureBytesPerPixel(textureFormat);

			for (mu_uint32 l = 0; l < numLevels; ++l)
			{
				Diligent::TextureSubResData resource;
				resource.pData = data[l];
				resource.Stride = std::max(width >> l, 1u) * bytesPerPixel;
				subresources.push_back(resource);
			}
		}

		Diligent::TextureDesc textureDesc;
#if NEXTMU_COMPILE_DEBUG == 1
		textureDesc.Name = label;
#endif
		textureDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = numLevels;
		textureDesc.Format = textureFormat;
		textureDesc.Usage = data != nullptr ? Diligent::USAGE_IMMUTABLE : Diligent::USAGE_DEFAULT;
		textureDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
		textureDesc.SampleCount = 1;

		Diligent::TextureData textureData;
		textureData.NumSubresources = static_cast<mu_uint32>(subresources.size());
		textureData.pSubResources = subresources.data();

		Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
		device->CreateTexture(textureDesc, textureData.NumSubresources > 0 ? &textureData : nullptr, &texture);
		if (texture == nullptr)
		{
			return nullptr;
		}

		return Noesis::MakePtr<DETexture>(
			width,
			height,
			numLevels,
			1,
			true,
			true,
			textureFormat,
			texture,
			data != nullptr
		);
	}

	static mu_uint32 Align(mu_uint32 n, mu_uint32 alignment)
	{
		return (n + (alignment - 1)) & ~(alignment - 1);
	}

	/// Updates texture mipmap copying the given data to desired position. The passed data is
	/// tightly packed (no extra pitch) and is never greater than DYNAMIC_TEX_SIZE bytes.
	/// Origin is located at the left of the first scanline
	void DERenderDevice::UpdateTexture(Noesis::Texture *texture, uint32_t level, uint32_t x, uint32_t y,
					   uint32_t width, uint32_t height, const void *data)
	{
		auto texture_ = static_cast<DETexture *>(texture);
		const auto bytesPerPixel = GetTextureBytesPerPixel(texture_->Format);
		const auto stride = Align(width * bytesPerPixel, 4);
		const auto immediateContext = MUGraphics::GetImmediateContext();

		void *srcData = const_cast<void*>(data);
		if (height > 1)
		{
			srcData = mu_malloc(stride * height);
			if (srcData == nullptr)
			{
				mu_error("failed to allocate temporary buffer to update texture");
				return;
			}

			const auto srcStride = width * bytesPerPixel;
			const mu_uint8 *src = reinterpret_cast<const mu_uint8 *>(data);
			mu_uint8 *dest = reinterpret_cast<mu_uint8 *>(srcData);
			for (mu_uint32 h = 0; h < height; ++h)
			{
				mu_memcpy(dest, src, srcStride);
				src += srcStride;
				dest += stride;
			}
		}

		Diligent::StateTransitionDesc transition(texture_->GetTexture(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
		immediateContext->TransitionResourceStates(1, &transition);
		immediateContext->UpdateTexture(
			texture_->Texture,
			level,
			0,
			Diligent::Box(x, x + width, y, y + height),
			Diligent::TextureSubResData(srcData, stride),
			Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE,
			texture_->GetRequireTransition() == false ? Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION : Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY
		);
		texture_->SetRequireTransition(true);

		if (height > 1)
		{
			mu_free(srcData);
		}
	}

	/// Begins rendering offscreen commands
	void DERenderDevice::BeginOffscreenRender()
	{
		// Nothing to do
		// Begin to render on a framebuffer
	}

	/// Ends rendering offscreen commands
	void DERenderDevice::EndOffscreenRender()
	{
		// Nothing to do
	}

	/// Begins rendering onscreen commands
	void DERenderDevice::BeginOnscreenRender()
	{
		// Nothing to do
		// Begin to render on screen
		const auto swapchain = MUGraphics::GetSwapChain();
		const auto &swapchainDesc = swapchain->GetDesc();
		FixedPipelineState.RTVFormat = swapchainDesc.ColorBufferFormat;
		FixedPipelineState.DSVFormat = swapchainDesc.DepthBufferFormat;
	}

	/// Ends rendering onscreen commands
	void DERenderDevice::EndOnscreenRender()
	{
		// Nothing to do
		ResourceBindingsManager.MergeTemporaryShaderBindings();
	}

	/// Binds render target and sets viewport to cover the entire surface. The existing contents of
	/// the surface are discarded and replaced with arbitrary data. Surface is not cleared
	void DERenderDevice::SetRenderTarget(Noesis::RenderTarget *surface)
	{
		const auto immediateContext = MUGraphics::GetImmediateContext();
		auto renderTarget_ = static_cast<DERenderTarget *>(surface);
		auto texture_ = static_cast<DETexture *>(renderTarget_->GetTexture());

		RenderTarget = renderTarget_;
		auto color = RenderTarget->IsMSAA() ? RenderTarget->ColorMSAATexture.RawPtr() : RenderTarget->ColorTexture->GetTexture();
		auto depthStencil = RenderTarget->HasDepthStencil() ? RenderTarget->DepthStencilTexture.RawPtr() : nullptr;
		Diligent::ITextureView *textureViews[1] = { color->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET) };
		immediateContext->SetRenderTargets(1, textureViews, depthStencil != nullptr ? depthStencil->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET) : nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		FixedPipelineState.RTVFormat = color->GetDesc().Format;
		FixedPipelineState.DSVFormat = depthStencil != nullptr ? depthStencil->GetDesc().Format : Diligent::TEX_FORMAT_UNKNOWN;
	}

	/// Indicates that until the next call to EndTile(), all drawing commands will only update the
	/// contents of the render target defined by the extension of the given tile. This is a good
	/// place to enable scissoring and apply optimizations for tile-based GPU architectures.
	void DERenderDevice::BeginTile(Noesis::RenderTarget* surface, const Noesis::Tile& tile)
	{
		const auto immediateContext = MUGraphics::GetImmediateContext();
		const auto deviceType = MUGraphics::GetDeviceType();

		auto renderTarget_ = static_cast<DERenderTarget *>(surface);
		auto texture_ = static_cast<DETexture *>(renderTarget_->GetTexture());

		Diligent::Rect scissorRect = (
			deviceType == Diligent::RENDER_DEVICE_TYPE_GL || Diligent::RENDER_DEVICE_TYPE_GLES
			? Diligent::Rect(tile.x, tile.y, tile.x + tile.width, tile.y + tile.height)
			: Diligent::Rect(tile.x, texture_->GetHeight() - (tile.y + tile.height), tile.x + tile.width, texture_->GetHeight() - tile.y)
		);
		immediateContext->SetScissorRects(1, &scissorRect, texture_->GetWidth(), texture_->GetHeight());
		EnableScissors = true;
	}

	/// Completes rendering to the tile specified by BeginTile()
	void DERenderDevice::EndTile(Noesis::RenderTarget* surface)
	{
		// Just disable the rasterizer flag
		EnableScissors = false;
		/*const auto immediateContext = MUGraphics::GetImmediateContext();
		auto renderTarget_ = static_cast<DERenderTarget *>(surface);
		auto texture_ = static_cast<DETexture *>(renderTarget_->GetTexture());
		immediateContext->SetScissorRects(0, nullptr, texture_->GetWidth(), texture_->GetHeight());*/
	}

	/// Resolves multisample render target. Transient surfaces (stencil, colorAA) are discarded.
	/// Only the specified list of surface regions are resolved
	void DERenderDevice::ResolveRenderTarget(Noesis::RenderTarget *surface, const Noesis::Tile *tiles, uint32_t numTiles)
	{
		auto renderTarget = static_cast<DERenderTarget *>(surface);
		if (renderTarget->ColorMSAATexture == nullptr) return;
		auto texture = static_cast<DETexture *>(renderTarget->GetTexture());

		const auto immediateContext = MUGraphics::GetImmediateContext();
		Diligent::ResolveTextureSubresourceAttribs ResolveAttribs;
		ResolveAttribs.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
		ResolveAttribs.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
		ResolveAttribs.Format = texture->Format;
		immediateContext->ResolveTextureSubresource(renderTarget->ColorMSAATexture, texture->GetTexture(), ResolveAttribs);
	}

	/// Gets a pointer to stream vertices (bytes <= DYNAMIC_VB_SIZE)
	void *DERenderDevice::MapVertices(uint32_t bytes)
	{
		return VertexBuffer.data();
	}

	/// Invalidates the pointer previously mapped
	void DERenderDevice::UnmapVertices()
	{
	}

	/// Gets a pointer to stream 16-bit indices (bytes <= DYNAMIC_IB_SIZE)
	void *DERenderDevice::MapIndices(uint32_t bytes)
	{
		return IndexBuffer.data();
	}

	/// Invalidates the pointer previously mapped
	void DERenderDevice::UnmapIndices()
	{
	}

	DETexture *DERenderDevice::GetTexture(Noesis::Texture *texture)
	{
		if (texture == nullptr) return DummyTexture.GetPtr();
		return static_cast<DETexture *>(texture);
	}

	NSampler *DERenderDevice::GetTextureSampler(DETexture *texture, const Noesis::SamplerState &sampler)
	{
		if (texture == nullptr) return nullptr;

		Diligent::SamplerDesc samplerDesc;

		switch (sampler.f.wrapMode)
		{
		case Noesis::WrapMode::ClampToEdge:
			samplerDesc.AddressU = Diligent::TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = Diligent::TEXTURE_ADDRESS_CLAMP;
			break;
		case Noesis::WrapMode::ClampToZero:
			samplerDesc.AddressU = Diligent::TEXTURE_ADDRESS_BORDER;
			samplerDesc.AddressV = Diligent::TEXTURE_ADDRESS_BORDER;
			break;
		case Noesis::WrapMode::Repeat:
			samplerDesc.AddressU = Diligent::TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = Diligent::TEXTURE_ADDRESS_WRAP;
			break;
		case Noesis::WrapMode::MirrorU:
			samplerDesc.AddressU = Diligent::TEXTURE_ADDRESS_MIRROR;
			samplerDesc.AddressV = Diligent::TEXTURE_ADDRESS_WRAP;
			break;
		case Noesis::WrapMode::MirrorV:
			samplerDesc.AddressU = Diligent::TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = Diligent::TEXTURE_ADDRESS_MIRROR;
			break;
		case Noesis::WrapMode::Mirror:
			samplerDesc.AddressU = Diligent::TEXTURE_ADDRESS_MIRROR;
			samplerDesc.AddressV = Diligent::TEXTURE_ADDRESS_MIRROR;
			break;
		}

		switch (sampler.f.minmagFilter)
		{
		case Noesis::MinMagFilter::Nearest:
			samplerDesc.MinFilter = Diligent::FILTER_TYPE_POINT;
			samplerDesc.MagFilter = Diligent::FILTER_TYPE_POINT;
			break;
		case Noesis::MinMagFilter::Linear:
			samplerDesc.MinFilter = Diligent::FILTER_TYPE_LINEAR;
			samplerDesc.MagFilter = Diligent::FILTER_TYPE_LINEAR;
			break;
		}

		switch (sampler.f.mipFilter)
		{
		case Noesis::MipFilter::Disabled:
			samplerDesc.MaxLOD = 0.0f;
		case Noesis::MipFilter::Nearest:
			samplerDesc.MipFilter = Diligent::FILTER_TYPE_POINT;
			break;
		case Noesis::MipFilter::Linear:
			samplerDesc.MipFilter = Diligent::FILTER_TYPE_LINEAR;
			break;
		}

		return ::GetTextureSampler(samplerDesc);
	}

	Diligent::ITextureView *DERenderDevice::GetTextureView(DETexture *texture)
	{
		if (texture == nullptr) return nullptr;
		return texture->GetTexture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
	}

	NResourceId DERenderDevice::GetTextureResourceId(DETexture *texture)
	{
		if (texture == nullptr) return NInvalidUInt32;
		return texture->ResourceId;
	}

	NResourceId DERenderDevice::GetTextureSamplerResourceId(NSampler *sampler)
	{
		if (IsCombinedSampler == true || sampler == nullptr) return NInvalidUInt32;
		return sampler->Id;
	}

	void DERenderDevice::GetDynamicPipelineState(const Noesis::Batch &batch, NDynamicPipelineState &dynamicState)
	{
		const auto immediateContext = MUGraphics::GetImmediateContext();

		auto &state = batch.renderState;
		dynamicState.WireframeMode = state.f.wireframe;
		dynamicState.CullMode = Diligent::CULL_MODE_NONE;
		dynamicState.ColorWrite = state.f.colorEnable > 0;
		dynamicState.AlphaWrite = state.f.colorEnable > 0;
		dynamicState.DepthWrite = false;
		dynamicState.DepthFunc = Diligent::COMPARISON_FUNC_UNKNOWN;

		switch (state.f.blendMode)
		{
		case Noesis::BlendMode::Src: break;
		case Noesis::BlendMode::SrcOver:
			{
				dynamicState.SrcBlend = Diligent::BLEND_FACTOR_ONE;
				dynamicState.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
				dynamicState.SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
				dynamicState.DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
			}
			break;
		case Noesis::BlendMode::SrcOver_Multiply:
			{
				dynamicState.SrcBlend = Diligent::BLEND_FACTOR_DEST_COLOR;
				dynamicState.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
				dynamicState.SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
				dynamicState.DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
			}
			break;
		case Noesis::BlendMode::SrcOver_Screen:
			{
				dynamicState.SrcBlend = Diligent::BLEND_FACTOR_ONE;
				dynamicState.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_COLOR;
				dynamicState.SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
				dynamicState.DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
			}
			break;
		case Noesis::BlendMode::SrcOver_Additive:
			{
				dynamicState.SrcBlend = Diligent::BLEND_FACTOR_ONE;
				dynamicState.DestBlend = Diligent::BLEND_FACTOR_ONE;
				dynamicState.SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
				dynamicState.DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
			}
			break;
		case Noesis::BlendMode::SrcOver_Dual:
			{
				dynamicState.SrcBlend = Diligent::BLEND_FACTOR_ONE;
				dynamicState.DestBlend = Diligent::BLEND_FACTOR_INV_SRC1_COLOR;
				dynamicState.SrcBlendAlpha = Diligent::BLEND_FACTOR_ONE;
				dynamicState.DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC1_ALPHA;
			}
			break;
		}

		switch (state.f.stencilMode)
		{
		case Noesis::StencilMode::Disabled: dynamicState.StencilEnable = false; break;
		case Noesis::StencilMode::Equal_Keep:
			dynamicState.StencilEnable = true;
			dynamicState.StencilFailOp = Diligent::STENCIL_OP_KEEP;
			dynamicState.StencilDepthFailOp = Diligent::STENCIL_OP_KEEP;
			dynamicState.StencilPassOp = Diligent::STENCIL_OP_KEEP;
			dynamicState.StencilFunc = Diligent::COMPARISON_FUNC_EQUAL;
			dynamicState.StencilReadMask = 0xFF;
			//dynamicState.StencilWriteMask = 0xFF;
			break;
		case Noesis::StencilMode::Equal_Incr:
			dynamicState.StencilEnable = true;
			dynamicState.StencilFailOp = Diligent::STENCIL_OP_KEEP;
			dynamicState.StencilDepthFailOp = Diligent::STENCIL_OP_KEEP;
			dynamicState.StencilPassOp = Diligent::STENCIL_OP_INCR_SAT;
			dynamicState.StencilFunc = Diligent::COMPARISON_FUNC_EQUAL;
			dynamicState.StencilReadMask = 0xFF;
			//dynamicState.StencilWriteMask = 0xFF;
			break;
		case Noesis::StencilMode::Equal_Decr:
			dynamicState.StencilEnable = true;
			dynamicState.StencilFailOp = Diligent::STENCIL_OP_KEEP;
			dynamicState.StencilDepthFailOp = Diligent::STENCIL_OP_KEEP;
			dynamicState.StencilPassOp = Diligent::STENCIL_OP_DECR_SAT;
			dynamicState.StencilFunc = Diligent::COMPARISON_FUNC_EQUAL;
			dynamicState.StencilReadMask = 0xFF;
			//dynamicState.StencilWriteMask = 0xFF;
			break;
		case Noesis::StencilMode::Clear:
			dynamicState.StencilEnable = true;
			dynamicState.StencilFailOp = Diligent::STENCIL_OP_ZERO;
			dynamicState.StencilDepthFailOp = Diligent::STENCIL_OP_ZERO;
			dynamicState.StencilPassOp = Diligent::STENCIL_OP_ZERO;
			dynamicState.StencilFunc = Diligent::COMPARISON_FUNC_ALWAYS;
			dynamicState.StencilReadMask = 0xFF;
			//dynamicState.StencilWriteMask = 0xFF;
			break;
		case Noesis::StencilMode::Disabled_ZTest:
			dynamicState.StencilEnable = false;
			dynamicState.DepthFunc = Diligent::COMPARISON_FUNC_LESS_EQUAL;
			break;
		case Noesis::StencilMode::Equal_Keep_ZTest:
			dynamicState.DepthFunc = Diligent::COMPARISON_FUNC_LESS_EQUAL;
			dynamicState.StencilEnable = true;
			dynamicState.StencilFailOp = Diligent::STENCIL_OP_KEEP;
			dynamicState.StencilDepthFailOp = Diligent::STENCIL_OP_KEEP;
			dynamicState.StencilPassOp = Diligent::STENCIL_OP_KEEP;
			dynamicState.StencilFunc = Diligent::COMPARISON_FUNC_EQUAL;
			dynamicState.StencilReadMask = 0xFF;
			//dynamicState.StencilWriteMask = 0xFF;
			break;
		}
	}

	/// Draws primitives for the given batch
	void DERenderDevice::DrawBatch(const Noesis::Batch &batch)
	{
		const auto immediateContext = MUGraphics::GetImmediateContext();

		mu_assert(batch.pixelShader == nullptr);
		auto shader = static_cast<Noesis::Shader::Enum>(batch.shader.v);
		auto vertexShader = Noesis::VertexForShader[shader];

		EnsureProgram(shader);
		const mu_shader program = Programs[shader];
		if (program == NInvalidShader) return;

		auto format = Noesis::FormatForVertex[vertexShader];
		mu_uint32 vertexSize = static_cast<mu_uint32>(Noesis::SizeForFormat[format]);
		auto &vertexBuffer = VertexBuffer;
		auto &indexBuffer = IndexBuffer;

		immediateContext->UpdateBuffer(
			GPUVertexBuffer,
			batch.vertexOffset,
			batch.numVertices * vertexSize,
			VertexBuffer.data() + batch.vertexOffset,
			Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
		);
		immediateContext->UpdateBuffer(
			GPUIndexBuffer,
			batch.startIndex * sizeof(mu_uint16),
			batch.numIndices * sizeof(mu_uint16),
			IndexBuffer.data() + batch.startIndex * sizeof(mu_uint16),
			Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
		);

		if (batch.vertexUniforms[0].values != nullptr)
		{
			Diligent::MapHelper<mu_float[1]> uniform(immediateContext, VertexUniforms[0], Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			mu_memcpy(*uniform, batch.vertexUniforms[0].values, sizeof(mu_float) * batch.vertexUniforms[0].numDwords);
		}

		if (batch.vertexUniforms[1].values != nullptr)
		{
			Diligent::MapHelper<mu_float[1]> uniform(immediateContext, VertexUniforms[1], Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			mu_memcpy(*uniform, batch.vertexUniforms[1].values, sizeof(mu_float) * batch.vertexUniforms[1].numDwords);
		}

		if (batch.pixelUniforms[0].values != nullptr)
		{
			Diligent::MapHelper<mu_float[1]> uniform(immediateContext, FragmentUniforms[0], Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			mu_memcpy(*uniform, batch.pixelUniforms[0].values, sizeof(mu_float) * batch.pixelUniforms[0].numDwords);
		}

		if (batch.pixelUniforms[1].values != nullptr)
		{
			Diligent::MapHelper<mu_float[1]> uniform(immediateContext, FragmentUniforms[1], Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			mu_memcpy(*uniform, batch.pixelUniforms[1].values, sizeof(mu_float) * batch.pixelUniforms[1].numDwords);
		}

		FixedPipelineState.CombinedShader = program;

		NDynamicPipelineState dynamicState;
		GetDynamicPipelineState(batch, dynamicState);
		if (batch.renderState.f.stencilMode != Noesis::StencilMode::Disabled) immediateContext->SetStencilRef(batch.stencilRef);

		auto pipelineState = GetPipelineState(FixedPipelineState, dynamicState);
		pipelineState->StaticInitialized = true;

		DETexture *textures[] = {
			GetTexture(batch.pattern),
			GetTexture(batch.ramps),
			GetTexture(batch.image),
			GetTexture(batch.glyphs),
			GetTexture(batch.shadow),
		};

		std::vector<Diligent::StateTransitionDesc> barriers;
		for (mu_uint32 n = 0; n < mu_countof(textures); ++n)
		{
			auto texture = textures[n];
			if (texture == nullptr || texture->GetRequireTransition() == false) continue;
			texture->SetRequireTransition(false);
			barriers.push_back(Diligent::StateTransitionDesc(texture->GetTexture(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE));
		}
		if (barriers.empty() == false) immediateContext->TransitionResourceStates(static_cast<mu_uint32>(barriers.size()), barriers.data());

		NSampler *samplers[] = {
			GetTextureSampler(textures[0], batch.patternSampler),
			GetTextureSampler(textures[1], batch.rampsSampler),
			GetTextureSampler(textures[2], batch.imageSampler),
			GetTextureSampler(textures[3], batch.glyphsSampler),
			GetTextureSampler(textures[4], batch.shadowSampler),
		};

		Diligent::ITextureView *textureViews[] = {
			GetTextureView(textures[0]),
			GetTextureView(textures[1]),
			GetTextureView(textures[2]),
			GetTextureView(textures[3]),
			GetTextureView(textures[4]),
		};

		if (IsCombinedSampler == true)
		{
			if (textureViews[0] != nullptr) textureViews[0]->SetSampler(samplers[0]->Sampler);
			if (textureViews[1] != nullptr) textureViews[1]->SetSampler(samplers[1]->Sampler);
			if (textureViews[2] != nullptr) textureViews[2]->SetSampler(samplers[2]->Sampler);
			if (textureViews[3] != nullptr) textureViews[3]->SetSampler(samplers[3]->Sampler);
			if (textureViews[4] != nullptr) textureViews[4]->SetSampler(samplers[4]->Sampler);
		}

		NResourceId resources[] = {
			GetTextureResourceId(textures[0]),
			GetTextureResourceId(textures[1]),
			GetTextureResourceId(textures[2]),
			GetTextureResourceId(textures[3]),
			GetTextureResourceId(textures[4]),
			GetTextureSamplerResourceId(samplers[0]),
			GetTextureSamplerResourceId(samplers[1]),
			GetTextureSamplerResourceId(samplers[2]),
			GetTextureSamplerResourceId(samplers[3]),
			GetTextureSamplerResourceId(samplers[4])
		};
		auto binding = ResourceBindingsManager.GetShaderBinding(0, ResourceSignature.RawPtr(), mu_countof(resources), resources);
		if (binding->Initialized == false)
		{
			//batch.pattern
			{
				if (!IsCombinedSampler)
					binding->Binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Pattern_sampler")->Set(samplers[0]->Sampler);
				binding->Binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Pattern")->Set(textureViews[0]);
			}

			//batch.ramps
			{
				if (!IsCombinedSampler)
					binding->Binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Ramps_sampler")->Set(samplers[1]->Sampler);
				binding->Binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Ramps")->Set(textureViews[1]);
			}
			
			//batch.image
			{
				if (!IsCombinedSampler)
					binding->Binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Image_sampler")->Set(samplers[2]->Sampler);
				binding->Binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Image")->Set(textureViews[2]);
			}
			
			//batch.glyphs
			{
				if (!IsCombinedSampler)
					binding->Binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Glyphs_sampler")->Set(samplers[3]->Sampler);
				binding->Binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Glyphs")->Set(textureViews[3]);
			}
			
			//batch.shadow
			{
				if (!IsCombinedSampler)
					binding->Binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Shadow_sampler")->Set(samplers[4]->Sampler);
				binding->Binding->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Shadow")->Set(textureViews[4]);
			}

			binding->Initialized = true;
		}

		Diligent::Uint64 vertexOffset = batch.vertexOffset;
		immediateContext->SetVertexBuffers(0, 1, &GPUVertexBuffer, &vertexOffset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		immediateContext->SetIndexBuffer(GPUIndexBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		immediateContext->SetPipelineState(pipelineState->Pipeline);
		auto stateTransitionMode = (
			binding->ShouldTransition
			? Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
			: Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY
		);
		binding->ShouldTransition = false;
		immediateContext->CommitShaderResources(binding->Binding, stateTransitionMode);
		immediateContext->DrawIndexed(
			Diligent::DrawIndexedAttribs(
				batch.numIndices,
				Diligent::VT_UINT16,
				Diligent::DRAW_FLAG_VERIFY_ALL,
				1,
				batch.startIndex
			)
		);
	}
};