#ifndef __UI_NOESISGUI_BGFX_H__
#define __UI_NOESISGUI_BGFX_H__

#pragma once

#include <queue>

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	namespace TextureUnit
	{
		enum : mu_uint8
		{
			Pattern,
			Ramps,
			Image,
			Glyphs,
			Shadow,
			Count,
		};
	};

	class BGFXRenderTarget;
	class BGFXTexture;

	struct BGFXViewport
	{
		mu_uint32 x = 0u;
		mu_uint32 y = 0u;
		mu_uint32 width = 0u;
		mu_uint32 height = 0u;
	};

	class BGFXVertexLayout
	{
	public:
		~BGFXVertexLayout()
		{
			if (bgfx::isValid(Handle))
			{
				bgfx::destroy(Handle);
				Handle = BGFX_INVALID_HANDLE;
			}
		}

	public:
		bgfx::VertexLayout Layout;
		bgfx::VertexLayoutHandle Handle;
	};
	
	class BGFXTextureSampler
	{
	public:
		~BGFXTextureSampler()
		{
			if (bgfx::isValid(Handle))
			{
				bgfx::destroy(Handle);
				Handle = BGFX_INVALID_HANDLE;
			}
		}

		bgfx::UniformHandle Handle = BGFX_INVALID_HANDLE;
	};

	class BGFXUniform
	{
	public:
		~BGFXUniform()
		{
			if (bgfx::isValid(Handle))
			{
				bgfx::destroy(Handle);
				Handle = BGFX_INVALID_HANDLE;
			}
		}

		bgfx::UniformHandle Handle = BGFX_INVALID_HANDLE;
	};

	class BGFXRenderDevice : public Noesis::RenderDevice
	{
	public:
		BGFXRenderDevice(const mu_boolean sRGB);

	private:
		void FillCaps(const mu_boolean sRGB);
		void CreateShaders();
		void CreateSamplers();
		void CreateUniforms();
		void CreateBuffers();

		const bgfx::ProgramHandle LoadShader(const Noesis::Shader::Enum shader);
		void EnsureProgram(const Noesis::Shader::Enum shader);

	public:
		/// Retrieves device render capabilities
		virtual const Noesis::DeviceCaps &GetCaps() const override;

		/// Creates render target surface with given dimensions, samples and optional stencil buffer
		virtual Noesis::Ptr<Noesis::RenderTarget> CreateRenderTarget(const char *label, uint32_t width, uint32_t height,
													 uint32_t sampleCount, bool needsStencil) override;

		/// Creates render target sharing transient (stencil, colorAA) buffers with the given surface
		virtual Noesis::Ptr<Noesis::RenderTarget> CloneRenderTarget(const char *label, Noesis::RenderTarget *surface) override;

		/// Creates texture with given dimensions and format. For immutable textures, the content of
		/// each mipmap is given in 'data'. The passed data is tightly packed (no extra pitch). When
		/// 'data' is null the texture is considered dynamic and will be updated using UpdateTexture()
		virtual Noesis::Ptr<Noesis::Texture> CreateTexture(const char *label, uint32_t width, uint32_t height,
										   uint32_t numLevels, Noesis::TextureFormat::Enum format, const void **data) override;

		/// Updates texture mipmap copying the given data to desired position. The passed data is
		/// tightly packed (no extra pitch) and is never greater than DYNAMIC_TEX_SIZE bytes.
		/// Origin is located at the left of the first scanline
		virtual void UpdateTexture(Noesis::Texture *texture, uint32_t level, uint32_t x, uint32_t y,
								   uint32_t width, uint32_t height, const void *data) override;

		/// Begins rendering offscreen commands
		virtual void BeginOffscreenRender() override;

		/// Ends rendering offscreen commands
		virtual void EndOffscreenRender() override;

		/// Begins rendering onscreen commands
		virtual void BeginOnscreenRender() override;

		/// Ends rendering onscreen commands
		virtual void EndOnscreenRender() override;

		/// Binds render target and sets viewport to cover the entire surface. The existing contents of
		/// the surface are discarded and replaced with arbitrary data. Surface is not cleared
		virtual void SetRenderTarget(Noesis::RenderTarget* surface) override;

		/// Indicates that until the next call to EndTile(), all drawing commands will only update the
		/// contents of the render target defined by the extension of the given tile. This is a good
		/// place to enable scissoring and apply optimizations for tile-based GPU architectures.
		virtual void BeginTile(Noesis::RenderTarget* surface, const Noesis::Tile& tile) override;

		/// Completes rendering to the tile specified by BeginTile()
		virtual void EndTile(Noesis::RenderTarget* surface) override;

		/// Resolves multisample render target. Transient surfaces (stencil, colorAA) are discarded.
		/// Only the specified list of surface regions are resolved
		virtual void ResolveRenderTarget(Noesis::RenderTarget *surface, const Noesis::Tile *tiles, uint32_t numTiles) override;

		/// Gets a pointer to stream vertices (bytes <= DYNAMIC_VB_SIZE)
		virtual void *MapVertices(uint32_t bytes) override;

		/// Invalidates the pointer previously mapped
		virtual void UnmapVertices() override;

		/// Gets a pointer to stream 16-bit indices (bytes <= DYNAMIC_IB_SIZE)
		virtual void *MapIndices(uint32_t bytes) override;

		/// Invalidates the pointer previously mapped
		virtual void UnmapIndices() override;

		void SetTexture(const mu_uint8 stage, BGFXTexture *texture, Noesis::SamplerState sampler);
		void SetTextures(const Noesis::Batch &batch);
		void SetRenderState(Noesis::RenderState state, uint8_t stencilRef);

		/// Draws primitives for the given batch
		virtual void DrawBatch(const Noesis::Batch &batch) override;

	private:
		uint16_t Scissor = bgfx::kInvalidHandle;
		std::vector<bgfx::ProgramHandle> Programs = std::vector<bgfx::ProgramHandle>(static_cast<size_t>(Noesis::Shader::Count), BGFX_INVALID_HANDLE);
		Noesis::DeviceCaps Caps;
		bgfx::ViewId RenderView = 0, NextRenderView = 0;
		BGFXRenderTarget *RenderTarget = nullptr;
		BGFXViewport RenderViewport;
		std::array<BGFXVertexLayout, Noesis::Shader::Vertex::Format::Count> VertexLayouts;
		mu_float TextureFlipped[TextureUnit::Count] = {};
		BGFXTextureSampler TextureSamplers[TextureUnit::Count];
		BGFXUniform VertexUniforms[2];
		BGFXUniform FragmentUniforms[2];
		BGFXUniform TexturesUniform;

		std::array<mu_uint8, DYNAMIC_VB_SIZE> VertexBuffer;
		std::array<mu_uint8, DYNAMIC_IB_SIZE> IndexBuffer;
	};
};
#endif

#endif