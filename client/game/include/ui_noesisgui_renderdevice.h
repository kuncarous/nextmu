#ifndef __UI_NOESISGUI_RENDERDEVICE_H__
#define __UI_NOESISGUI_RENDERDEVICE_H__

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

	class DERenderTarget;
	class DETexture;

	class DERenderDevice : public Noesis::RenderDevice
	{
	public:
		DERenderDevice(const mu_boolean sRGB);
		~DERenderDevice();

	public:
		void ResetShaders();

	private:
		void FillCaps(const mu_boolean sRGB);
		void CreateShaderResources();
		void InitializeShaderResources();
		void CreateShaders();
		void CreateUniforms();
		void CreateBuffers();

		const mu_shader LoadShader(const Noesis::Shader::Enum shader);
		void EnsureProgram(const Noesis::Shader::Enum shader);

	public:
		/// Retrieves device render capabilities
		virtual const Noesis::DeviceCaps &GetCaps() const override;

		Noesis::Ptr<Noesis::RenderTarget> CreateRenderTarget(
			const char *label,
			uint32_t width, uint32_t height,
			uint32_t sampleCount, bool needsStencil,
			Diligent::RefCntAutoPtr<Diligent::ITexture> colorMSAA,
			Diligent::RefCntAutoPtr<Diligent::ITexture> stencil
		);

		/// Creates render target surface with given dimensions, samples and optional stencil buffer
		virtual Noesis::Ptr<Noesis::RenderTarget> CreateRenderTarget(const char *label, uint32_t width, uint32_t height, uint32_t sampleCount, bool needsStencil) override;

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

		DETexture *GetTexture(Noesis::Texture *texture);
		NSampler *GetTextureSampler(DETexture *texture, const Noesis::SamplerState &sampler);
		Diligent::ITextureView *GetTextureView(DETexture *texture);
		NResourceId GetTextureResourceId(DETexture *texture);
		NResourceId GetTextureSamplerResourceId(NSampler *sampler);
		void GetDynamicPipelineState(const Noesis::Batch &batch, NDynamicPipelineState &dynamicState);

		/// Draws primitives for the given batch
		virtual void DrawBatch(const Noesis::Batch &batch) override;

	private:
		NFixedPipelineState FixedPipelineState;
		mu_boolean EnableScissors = false;
		mu_boolean IsCombinedSampler = false;
		std::vector<mu_shader> Programs;
		Noesis::DeviceCaps Caps;
		DERenderTarget *RenderTarget = nullptr;
		std::array<Diligent::InputLayoutDescX, Noesis::Shader::Vertex::Format::Count> InputLayouts;
		Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> ResourceSignature;
		NShaderResourcesBindingManager<Diligent::IPipelineResourceSignature> ResourceBindingsManager;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> GPUVertexBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> GPUIndexBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> VertexUniforms[2];
		Diligent::RefCntAutoPtr<Diligent::IBuffer> FragmentUniforms[2];

		std::array<mu_uint8, DYNAMIC_VB_SIZE> VertexBuffer;
		std::array<mu_uint8, DYNAMIC_IB_SIZE> IndexBuffer;
	};
};
#endif

#endif