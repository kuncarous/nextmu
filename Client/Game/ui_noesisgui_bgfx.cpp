#include "stdafx.h"
#include "ui_noesisgui_bgfx.h"
#include "ui_noesisgui_bgfxrendertarget.h"
#include "ui_noesisgui_bgfxtexture.h"
#include "ui_noesisgui_consts.h"
#include "mu_resourcesmanager.h"
#include "mu_renderstate.h"
#include "mu_config.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	bgfx::TextureFormat::Enum GetTextureFormat(Noesis::TextureFormat::Enum format)
	{
		switch (format)
		{
		default:
		case Noesis::TextureFormat::RGBA8: return bgfx::TextureFormat::RGBA8;
		case Noesis::TextureFormat::RGBX8: return bgfx::TextureFormat::RGB8;
		case Noesis::TextureFormat::R8: return bgfx::TextureFormat::R8;
		}
	}

	const mu_uint32 GetTextureBytesPerPixel(const bgfx::TextureFormat::Enum format)
	{
		switch (format)
		{
		default:
		case bgfx::TextureFormat::RGBA8: return 4;
		case bgfx::TextureFormat::RGB8: return 3;
		case bgfx::TextureFormat::R8: return 1;
		}
	}

	const mu_uint64 CalculateTextureSize(const bgfx::TextureFormat::Enum format, mu_uint32 width, mu_uint32 height, const mu_uint32 levels)
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

	bgfx::Attrib::Enum BgfxFormatType(uint32_t type)
	{
		switch (type)
		{
		case Noesis::Shader::Vertex::Format::Attr::Pos: return bgfx::Attrib::Position;
		case Noesis::Shader::Vertex::Format::Attr::Color: return bgfx::Attrib::Color0;
		case Noesis::Shader::Vertex::Format::Attr::Tex0: return bgfx::Attrib::TexCoord0;
		case Noesis::Shader::Vertex::Format::Attr::Tex1: return bgfx::Attrib::TexCoord1;
		case Noesis::Shader::Vertex::Format::Attr::Coverage: return bgfx::Attrib::TexCoord4;
		case Noesis::Shader::Vertex::Format::Attr::Rect: return bgfx::Attrib::TexCoord2;
		case Noesis::Shader::Vertex::Format::Attr::Tile: return bgfx::Attrib::TexCoord3;
		case Noesis::Shader::Vertex::Format::Attr::ImagePos: return bgfx::Attrib::TexCoord5;
		default: NS_ASSERT_UNREACHABLE;
		}
	}

	uint8_t BgfxAttribNum(uint32_t type)
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

	bgfx::AttribType::Enum BgfxAttribType(uint32_t type)
	{
		switch (type)
		{
		case Noesis::Shader::Vertex::Format::Attr::Type::Float: return bgfx::AttribType::Float;
		case Noesis::Shader::Vertex::Format::Attr::Type::Float2: return bgfx::AttribType::Float;
		case Noesis::Shader::Vertex::Format::Attr::Type::Float4: return bgfx::AttribType::Float;
		case Noesis::Shader::Vertex::Format::Attr::Type::UByte4Norm: return bgfx::AttribType::Uint8;
		case Noesis::Shader::Vertex::Format::Attr::Type::UShort4Norm: return bgfx::AttribType::Int16;
		default: NS_ASSERT_UNREACHABLE;
		}
	}

	mu_boolean BgfxAttribNormalized(uint32_t type)
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

	BGFXRenderDevice::BGFXRenderDevice(const mu_boolean sRGB)
	{
		FillCaps(sRGB);
		CreateShaders();
		CreateSamplers();
		CreateUniforms();
		CreateBuffers();
	}

	void BGFXRenderDevice::FillCaps(const mu_boolean sRGB)
	{
		const auto renderer = bgfx::getRendererType();
		Caps.centerPixelOffset = 0.0f;
		Caps.linearRendering = sRGB;
		Caps.subpixelRendering = (
			renderer == bgfx::RendererType::Direct3D11 ||
			renderer == bgfx::RendererType::Direct3D12 ||
			renderer == bgfx::RendererType::Vulkan ||
			renderer == bgfx::RendererType::Metal
		);
		Caps.depthRangeZeroToOne = (
			renderer != bgfx::RendererType::OpenGL &&
			renderer != bgfx::RendererType::OpenGLES
		);
	}

	void BGFXRenderDevice::CreateShaders()
	{
		for (mu_uint32 n = 0; n < Noesis::Shader::Vertex::Format::Count; ++n)
		{
			auto &layout = VertexLayouts[n];
			if (layout.Layout.m_stride == 0)
			{
				layout.Layout.begin(bgfx::RendererType::Direct3D11);
				uint32_t attributes = Noesis::AttributesForFormat[n];
				for (uint32_t j = 0; j < Noesis::Shader::Vertex::Format::Attr::Count; j++)
				{
					if ((attributes & (1 << j)) == 0) continue;
					uint8_t t = Noesis::TypeForAttr[j];
					layout.Layout.add(BgfxFormatType(j), BgfxAttribNum(t), BgfxAttribType(t), BgfxAttribNormalized(t));
				}
				layout.Layout.end();
				mu_assert(layout.Layout.m_stride == Noesis::SizeForFormat[n]);
			}

			layout.Handle = bgfx::createVertexLayout(layout.Layout);
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

	void BGFXRenderDevice::CreateSamplers()
	{
		static const mu_char *names[TextureUnit::Count] = {
			"pattern",
			"ramps",
			"image",
			"glyphs",
			"shadow",
		};
		for (mu_uint32 n = 0; n < static_cast<mu_uint32>(TextureUnit::Count); ++n)
		{
			auto &sampler = TextureSamplers[n];
			sampler.Handle = bgfx::createUniform(names[n], bgfx::UniformType::Sampler);
			mu_assert(bgfx::isValid(sampler.Handle) == true);
		}
	}

	void BGFXRenderDevice::CreateUniforms()
	{
		VertexUniforms[0].Handle = bgfx::createUniform("cbuffer0_vs", bgfx::UniformType::Mat4, 1);
		mu_assert(bgfx::isValid(VertexUniforms[0].Handle) == true);
		VertexUniforms[1].Handle = bgfx::createUniform("cbuffer1_vs", bgfx::UniformType::Vec4, 1);
		mu_assert(bgfx::isValid(VertexUniforms[1].Handle) == true);
		FragmentUniforms[0].Handle = bgfx::createUniform("cbuffer0_ps", bgfx::UniformType::Vec4, 2);
		mu_assert(bgfx::isValid(VertexUniforms[0].Handle) == true);
		FragmentUniforms[1].Handle = bgfx::createUniform("cbuffer1_ps", bgfx::UniformType::Vec4, 32);
		mu_assert(bgfx::isValid(VertexUniforms[1].Handle) == true);
	}

	void BGFXRenderDevice::CreateBuffers()
	{
		//auto &vertexLayout = DummyLayout;
		//VertexBuffer.Handle = bgfx::createDynamicVertexBuffer(static_cast<uint32_t>(DYNAMIC_VB_SIZE) / vertexLayout.getStride(), vertexLayout);
		//IndexBuffer.Handle = bgfx::createDynamicIndexBuffer(static_cast<uint32_t>(DYNAMIC_IB_SIZE) / sizeof(mu_uint16));
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

	const bgfx::ProgramHandle BGFXRenderDevice::LoadShader(const Noesis::Shader::Enum shader)
	{
		const mu_utf8string id = ShaderIDs[shader];
		const bgfx::ProgramHandle program = MUResourcesManager::GetProgram(id);
		if (bgfx::isValid(program))
		{
			return program;
		}

		const mu_utf8string path = mu_utf8string("./data/shaders/noesisgui/") + ShaderPaths[shader];
		if (MUResourcesManager::LoadProgram(id, path, path) == false)
		{
			return BGFX_INVALID_HANDLE;
		}

		return MUResourcesManager::GetProgram(id);
	}

	void BGFXRenderDevice::EnsureProgram(const Noesis::Shader::Enum shader)
	{
		if (bgfx::isValid(Programs[shader])) return;
		Programs[shader] = LoadShader(shader);
	}

	/// Retrieves device render capabilities
	const Noesis::DeviceCaps &BGFXRenderDevice::GetCaps() const
	{
		return Caps;
	}

	/// Creates render target surface with given dimensions, samples and optional stencil buffer
	Noesis::Ptr<Noesis::RenderTarget> BGFXRenderDevice::CreateRenderTarget(const char *label, uint32_t width, uint32_t height,
														 uint32_t sampleCount, bool needsStencil)
	{
		const auto caps = bgfx::getCaps();
		if (!caps) return nullptr;
		
		sampleCount = glm::min(sampleCount, 1u); // TODO : Implement MSAA support

		const mu_uint64 msaaFlags = (
			sampleCount >= 16
			? BGFX_TEXTURE_RT_MSAA_X16
			: sampleCount >= 8
			? BGFX_TEXTURE_RT_MSAA_X8
			: sampleCount >= 4
			? BGFX_TEXTURE_RT_MSAA_X4
			: sampleCount >= 2
			? BGFX_TEXTURE_RT_MSAA_X2
			: BGFX_TEXTURE_RT
		);
		const auto textureFormat = bgfx::TextureFormat::RGBA8;

		bgfx::TextureHandle color = bgfx::createTexture2D(
			width,
			height,
			false,
			1,
			textureFormat,
			(sampleCount > 1 ? BGFX_TEXTURE_BLIT_DST : BGFX_TEXTURE_RT) | BGFX_SAMPLER_POINT
		);
		if (bgfx::isValid(color) == false)
		{
			return nullptr;
		}

		bgfx::TextureHandle colorMSAA = BGFX_INVALID_HANDLE;
		if (sampleCount >= 2)
		{
			colorMSAA = bgfx::createTexture2D(
				width,
				height,
				false,
				1,
				textureFormat,
				msaaFlags | BGFX_SAMPLER_POINT
			);
			if (bgfx::isValid(colorMSAA) == false)
			{
				bgfx::destroy(color);
				return nullptr;
			}
		}

		bgfx::TextureHandle depthStencil = BGFX_INVALID_HANDLE;
		if (needsStencil)
		{
			depthStencil = bgfx::createTexture2D(
				width,
				height,
				false,
				1,
				bgfx::TextureFormat::D24S8,
				msaaFlags | BGFX_SAMPLER_NONE | BGFX_TEXTURE_RT_WRITE_ONLY
			);
			if (bgfx::isValid(depthStencil) == false)
			{
				if (sampleCount >= 2) bgfx::destroy(colorMSAA);
				bgfx::destroy(color);
				return nullptr;
			}
		}

		const bgfx::TextureHandle handles[2] = {
			bgfx::isValid(colorMSAA) ? colorMSAA : color,
			depthStencil
		};
		auto framebuffer = bgfx::createFrameBuffer(
			needsStencil ? 2 : 1,
			handles,
			false
		);
		if (bgfx::isValid(framebuffer) == false)
		{
			bgfx::destroy(color);
			bgfx::destroy(depthStencil);
			return nullptr;
		}

		return Noesis::MakePtr<BGFXRenderTarget>(
			Noesis::MakePtr<BGFXTexture>(
				width,
				height,
				1,
				sampleCount,
				textureFormat,
				false,
				true,
				color
			),
			colorMSAA,
			depthStencil,
			framebuffer
		);
	}

	/// Creates render target sharing transient (stencil, colorAA) buffers with the given surface
	Noesis::Ptr<Noesis::RenderTarget> BGFXRenderDevice::CloneRenderTarget(const char *label, Noesis::RenderTarget *surface)
	{
		BGFXRenderTarget *renderTarget = static_cast<BGFXRenderTarget *>(surface);
		BGFXTexture *texture = static_cast<BGFXTexture *>(renderTarget->GetTexture());
		return CreateRenderTarget(
			label,
			texture->GetWidth(),
			texture->GetHeight(),
			texture->GetSampleCount(),
			renderTarget->HasDepthStencil()
		);
	}

	/// Creates texture with given dimensions and format. For immutable textures, the content of
	/// each mipmap is given in 'data'. The passed data is tightly packed (no extra pitch). When
	/// 'data' is null the texture is considered dynamic and will be updated using UpdateTexture()
	Noesis::Ptr<Noesis::Texture> BGFXRenderDevice::CreateTexture(const char *label, uint32_t width, uint32_t height,
											   uint32_t numLevels, Noesis::TextureFormat::Enum format, const void **data)
	{
		const auto textureFormat = GetTextureFormat(format);
		const auto memorySize = CalculateTextureSize(textureFormat, width, height, numLevels);
		const bgfx::Memory *memory = !!data ? bgfx::alloc(static_cast<uint32_t>(memorySize)) : nullptr;
		if (!!data && !memory)
		{
			return nullptr;
		}

		if (memory != nullptr)
		{
			const auto bytesPerPixel = GetTextureBytesPerPixel(textureFormat);
			mu_uint8 *dest = memory->data;
			for (mu_uint32 l = 0, w = width, h = height; l < numLevels; ++l)
			{
				const mu_uint32 size = w * h * bytesPerPixel;
				mu_memcpy(dest, data[l], size);
				dest += size;
				w = std::max(w >> 1u, 1u);
				h = std::max(h >> 1u, 1u);
			}
		}

		bgfx::TextureHandle texture = bgfx::createTexture2D(
			static_cast<mu_uint16>(width),
			static_cast<mu_uint16>(height),
			numLevels > 1,
			1,
			textureFormat,
			BGFX_TEXTURE_NONE | BGFX_SAMPLER_POINT,
			memory
		);
		if (bgfx::isValid(texture) == false)
		{
			return nullptr;
		}

		return Noesis::MakePtr<BGFXTexture>(
			width,
			height,
			numLevels,
			1,
			textureFormat,
			true,
			true,
			texture
		);
	}

	/// Updates texture mipmap copying the given data to desired position. The passed data is
	/// tightly packed (no extra pitch) and is never greater than DYNAMIC_TEX_SIZE bytes.
	/// Origin is located at the left of the first scanline
	void BGFXRenderDevice::UpdateTexture(Noesis::Texture *texture, uint32_t level, uint32_t x, uint32_t y,
					   uint32_t width, uint32_t height, const void *data)
	{
		const auto bgfxTexture = static_cast<BGFXTexture *>(texture);
		const bgfx::Memory *memory = bgfx::copy(data, static_cast<mu_uint32>(CalculateTextureSize(bgfxTexture->Format, width, height, 1)));
		if (!memory) return;

		bgfx::updateTexture2D(
			bgfxTexture->Texture,
			0,
			level,
			x,
			y,
			width,
			height,
			memory
		);
	}

	/// Begins rendering offscreen commands
	void BGFXRenderDevice::BeginOffscreenRender()
	{
		// Nothing to do
		// Begin to render on a framebuffer
	}

	/// Ends rendering offscreen commands
	void BGFXRenderDevice::EndOffscreenRender()
	{
		// Nothing to do
		RenderTarget = nullptr;
	}

	/// Begins rendering onscreen commands
	void BGFXRenderDevice::BeginOnscreenRender()
	{
		// Nothing to do
		// Begin to render on screen
		RenderView = MURenderState::RenderView;
		bgfx::setViewRect(RenderView, 0, 0, MUConfig::GetWindowWidth(), MUConfig::GetWindowHeight());
		bgfx::setViewMode(RenderView, bgfx::ViewMode::Sequential);
	}

	/// Ends rendering onscreen commands
	void BGFXRenderDevice::EndOnscreenRender()
	{
		// Nothing to do
		bgfx::setViewMode(RenderView, bgfx::ViewMode::Default);
		RenderView = 0;
		NextRenderView = 0;
	}

	/// Binds render target and sets viewport to cover the entire surface. The existing contents of
	/// the surface are discarded and replaced with arbitrary data. Surface is not cleared
	void BGFXRenderDevice::SetRenderTarget(Noesis::RenderTarget *surface)
	{
		auto bgfxSurface = static_cast<BGFXRenderTarget *>(surface);
		auto bgfxTexture = static_cast<BGFXTexture *>(bgfxSurface->GetTexture());

		RenderTarget = bgfxSurface;

		RenderViewport.x = 0u;
		RenderViewport.y = 0u;
		RenderViewport.width = bgfxTexture->GetWidth();
		RenderViewport.height = bgfxTexture->GetHeight();

		RenderView = NextRenderView++;
		bgfx::setViewFrameBuffer(RenderView, bgfxSurface->Framebuffer);
		bgfx::setViewMode(RenderView, bgfx::ViewMode::Sequential);
		bgfx::setViewRect(RenderView, 0, 0, RenderViewport.width, RenderViewport.height);
		bgfx::setViewClear(RenderView, BGFX_CLEAR_NONE);
		bgfx::touch(RenderView);
	}

	/// Indicates that until the next call to EndTile(), all drawing commands will only update the
	/// contents of the render target defined by the extension of the given tile. This is a good
	/// place to enable scissoring and apply optimizations for tile-based GPU architectures.
	void BGFXRenderDevice::BeginTile(Noesis::RenderTarget* surface, const Noesis::Tile& tile)
	{
		auto texture = surface->GetTexture();
		Scissor = bgfx::setScissor(tile.x, tile.y, tile.width, tile.height);
	}

	/// Completes rendering to the tile specified by BeginTile()
	void BGFXRenderDevice::EndTile(Noesis::RenderTarget* surface)
	{
		Scissor = bgfx::kInvalidHandle;
	}

	/// Resolves multisample render target. Transient surfaces (stencil, colorAA) are discarded.
	/// Only the specified list of surface regions are resolved
	void BGFXRenderDevice::ResolveRenderTarget(Noesis::RenderTarget *surface, const Noesis::Tile *tiles, uint32_t numTiles)
	{
		auto bgfxSurface = static_cast<BGFXRenderTarget *>(surface);
		if (bgfx::isValid(bgfxSurface->ColorMSAATexture) == false) return;

		auto bgfxTexture = static_cast<BGFXTexture *>(bgfxSurface->GetTexture());

		for (mu_uint32 n = 0; n < numTiles; ++n)
		{
			const auto &tile = tiles[n];
			bgfx::blit(
				RenderView,
				bgfxTexture->Texture,
				tile.x,
				tile.y,
				bgfxSurface->ColorMSAATexture,
				tile.x,
				tile.y,
				tile.width,
				tile.height
			);
		}
	}

	/// Gets a pointer to stream vertices (bytes <= DYNAMIC_VB_SIZE)
	void *BGFXRenderDevice::MapVertices(uint32_t bytes)
	{
		mu_info("[MapVertices] {}", bytes);
		return VertexBuffer.data();
	}

	/// Invalidates the pointer previously mapped
	void BGFXRenderDevice::UnmapVertices()
	{
		mu_info("[UnmapVertices]");
	}

	/// Gets a pointer to stream 16-bit indices (bytes <= DYNAMIC_IB_SIZE)
	void *BGFXRenderDevice::MapIndices(uint32_t bytes)
	{
		mu_info("[MapIndices] {}", bytes);
		return IndexBuffer.data();
	}

	/// Invalidates the pointer previously mapped
	void BGFXRenderDevice::UnmapIndices()
	{
		mu_info("[UnmapIndices]");
	}

	void BGFXRenderDevice::SetTexture(const mu_uint8 stage, BGFXTexture *texture, Noesis::SamplerState sampler)
	{
		uint32_t flags = 0;
		switch (sampler.f.wrapMode)
		{
		case Noesis::WrapMode::ClampToEdge: flags |= BGFX_SAMPLER_UVW_CLAMP; break;
		case Noesis::WrapMode::ClampToZero: flags |= BGFX_SAMPLER_UVW_BORDER; break;
		case Noesis::WrapMode::Repeat: break;
		case Noesis::WrapMode::MirrorU: flags |= BGFX_SAMPLER_U_MIRROR; break;
		case Noesis::WrapMode::MirrorV: flags |= BGFX_SAMPLER_V_MIRROR; break;
		case Noesis::WrapMode::Mirror: flags |= BGFX_SAMPLER_UVW_MIRROR; break;
		}

		switch (sampler.f.minmagFilter)
		{
		case Noesis::MinMagFilter::Nearest: flags |= BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT; break;
		case Noesis::MinMagFilter::Linear: break;
		}

		switch (sampler.f.mipFilter)
		{
		case Noesis::MipFilter::Nearest: flags |= BGFX_SAMPLER_MIP_POINT; break;
		case Noesis::MipFilter::Linear: break;
		}

		bgfx::setTexture(stage, TextureSamplers[stage].Handle, texture->GetTexture(), flags);
	}

	void BGFXRenderDevice::SetTextures(const Noesis::Batch &batch)
	{
		if (batch.pattern != nullptr)
		{
			SetTexture(TextureUnit::Pattern, static_cast<BGFXTexture *>(batch.pattern), batch.patternSampler);
		}

		if (batch.ramps != nullptr)
		{
			SetTexture(TextureUnit::Ramps, static_cast<BGFXTexture *>(batch.ramps), batch.rampsSampler);
		}

		if (batch.image != nullptr)
		{
			SetTexture(TextureUnit::Image, static_cast<BGFXTexture *>(batch.image), batch.imageSampler);
		}

		if (batch.glyphs != nullptr)
		{
			SetTexture(TextureUnit::Glyphs, static_cast<BGFXTexture *>(batch.glyphs), batch.glyphsSampler);
		}

		if (batch.shadow != nullptr)
		{
			SetTexture(TextureUnit::Shadow, static_cast<BGFXTexture *>(batch.shadow), batch.shadowSampler);
		}
	}

	void BGFXRenderDevice::SetRenderState(Noesis::RenderState state, uint8_t stencilRef)
	{
		uint64_t bgfxState = BGFX_STATE_MSAA;

		if (state.f.colorEnable > 0)
			bgfxState |= BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A;

		switch (state.f.blendMode)
		{
		case Noesis::BlendMode::Src: break;
		case Noesis::BlendMode::SrcOver: bgfxState |= BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA, BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA); break;
		case Noesis::BlendMode::SrcOver_Multiply: bgfxState |= BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_INV_SRC_ALPHA, BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA); break;
		case Noesis::BlendMode::SrcOver_Screen: bgfxState |= BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_COLOR, BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA); break;
		case Noesis::BlendMode::SrcOver_Additive: bgfxState |= BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA); break;
		}

		switch (state.f.stencilMode)
		{
		case Noesis::StencilMode::Disabled: break;
		case Noesis::StencilMode::Equal_Keep:
			bgfx::setStencil(
				BGFX_STENCIL_OP_FAIL_S_KEEP |
				BGFX_STENCIL_OP_FAIL_Z_KEEP |
				BGFX_STENCIL_OP_PASS_Z_KEEP |
				BGFX_STENCIL_TEST_EQUAL |
				BGFX_STENCIL_FUNC_REF(stencilRef) |
				BGFX_STENCIL_FUNC_RMASK(255),
				BGFX_STENCIL_NONE
			);
			break;
		case Noesis::StencilMode::Equal_Incr:
			bgfx::setStencil(
				BGFX_STENCIL_OP_FAIL_S_KEEP |
				BGFX_STENCIL_OP_FAIL_Z_KEEP |
				BGFX_STENCIL_OP_PASS_Z_INCR |
				BGFX_STENCIL_TEST_EQUAL |
				BGFX_STENCIL_FUNC_REF(stencilRef) |
				BGFX_STENCIL_FUNC_RMASK(255),
				BGFX_STENCIL_NONE
			);
			break;
		case Noesis::StencilMode::Equal_Decr:
			bgfx::setStencil(
				BGFX_STENCIL_OP_FAIL_S_KEEP |
				BGFX_STENCIL_OP_FAIL_Z_KEEP |
				BGFX_STENCIL_OP_PASS_Z_DECR |
				BGFX_STENCIL_TEST_EQUAL |
				BGFX_STENCIL_FUNC_REF(stencilRef) |
				BGFX_STENCIL_FUNC_RMASK(255),
				BGFX_STENCIL_NONE
			);
			break;
		case Noesis::StencilMode::Clear:
			bgfx::setStencil(
				BGFX_STENCIL_OP_FAIL_S_ZERO |
				BGFX_STENCIL_OP_FAIL_Z_ZERO |
				BGFX_STENCIL_OP_PASS_Z_ZERO |
				BGFX_STENCIL_TEST_ALWAYS |
				BGFX_STENCIL_FUNC_REF(0) |
				BGFX_STENCIL_FUNC_RMASK(255),
				BGFX_STENCIL_NONE
			);
			break;
		case Noesis::StencilMode::Disabled_ZTest:
			bgfxState |= BGFX_STATE_DEPTH_TEST_LEQUAL;
			break;
		case Noesis::StencilMode::Equal_Keep_ZTest:
			bgfxState |= BGFX_STATE_DEPTH_TEST_LEQUAL;
			bgfx::setStencil(
				BGFX_STENCIL_OP_FAIL_S_KEEP |
				BGFX_STENCIL_OP_FAIL_Z_KEEP |
				BGFX_STENCIL_OP_PASS_Z_KEEP |
				BGFX_STENCIL_TEST_EQUAL |
				BGFX_STENCIL_FUNC_REF(stencilRef) |
				BGFX_STENCIL_FUNC_RMASK(255),
				BGFX_STENCIL_NONE
			);
			break;
		}

		bgfx::setState(
			bgfxState
		);
	}

	/// Draws primitives for the given batch
	void BGFXRenderDevice::DrawBatch(const Noesis::Batch &batch)
	{
		mu_info("[DrawBatch]");

		mu_assert(batch.pixelShader == nullptr);
		auto shader = static_cast<Noesis::Shader::Enum>(batch.shader.v);
		auto vertexShader = Noesis::VertexForShader[shader];

		EnsureProgram(shader);
		const bgfx::ProgramHandle program = Programs[shader];
		if (bgfx::isValid(program) == false) return;

		auto format = Noesis::FormatForVertex[vertexShader];
		auto stride = Noesis::SizeForFormat[format];
		auto &vertexBuffer = VertexBuffer;
		auto &indexBuffer = IndexBuffer;

		const auto &layout = VertexLayouts[format];
		bgfx::TransientVertexBuffer vb{};
		bgfx::allocTransientVertexBuffer(&vb, batch.numVertices, layout.Layout);
		mu_memcpy(vb.data, VertexBuffer.data() + batch.vertexOffset, batch.numVertices * layout.Layout.getStride());

		bgfx::TransientIndexBuffer ib{};
		bgfx::allocTransientIndexBuffer(&ib, batch.numIndices);
		mu_memcpy(ib.data, IndexBuffer.data() + batch.startIndex * sizeof(mu_uint16), batch.numIndices * sizeof(mu_uint16));

		SetRenderState(batch.renderState, batch.stencilRef);
		bgfx::setVertexBuffer(0, &vb, 0, batch.numVertices, layout.Handle);
		bgfx::setIndexBuffer(&ib, 0, batch.numIndices);
		SetTextures(batch);

		static mu_float buffer[128];
		if (batch.vertexUniforms[0].values != nullptr)
		{
			mu_memcpy(buffer, batch.vertexUniforms[0].values, sizeof(mu_float) * batch.vertexUniforms[0].numDwords);
			bgfx::setUniform(VertexUniforms[0].Handle, buffer);
		}

		if (batch.vertexUniforms[1].values != nullptr)
		{
			mu_memcpy(buffer, batch.vertexUniforms[1].values, sizeof(mu_float) * batch.vertexUniforms[1].numDwords);
			bgfx::setUniform(VertexUniforms[1].Handle, buffer);
		}

		if (batch.pixelUniforms[0].values != nullptr)
		{
			mu_memcpy(buffer, batch.pixelUniforms[0].values, sizeof(mu_float) * batch.pixelUniforms[0].numDwords);
			bgfx::setUniform(FragmentUniforms[0].Handle, buffer, (batch.pixelUniforms[0].numDwords / 4) + !!(batch.pixelUniforms[0].numDwords % 4));
		}

		if (batch.pixelUniforms[1].values != nullptr)
		{
			mu_memcpy(buffer, batch.pixelUniforms[1].values, sizeof(mu_float) * batch.pixelUniforms[1].numDwords);
			bgfx::setUniform(FragmentUniforms[1].Handle, buffer, (batch.pixelUniforms[1].numDwords / 4) + !!(batch.pixelUniforms[1].numDwords % 4));
		}

		if (Scissor != bgfx::kInvalidHandle)
		{
			bgfx::setScissor(Scissor);
		}

		bgfx::submit(RenderView, program);
	}
};
#endif