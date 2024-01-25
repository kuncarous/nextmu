#ifndef __UI_RMLUI_RENDERER_H__
#define __UI_RMLUI_RENDERER_H__

#pragma once

#include <RmlUi/Core/RenderInterface.h>
#include <SDL.h>

namespace UIRmlUI
{
	constexpr mu_uint32 StartVertexCount = 4096;
	constexpr mu_uint32 VertexCountMultiplier = 2;

	constexpr mu_uint32 StartIndexCount = 8192;
	constexpr mu_uint32 IndexCountMultiplier = 2;

	enum class ScissorMode
	{
		None,
		Normal,
		Stencil,
	};

	struct NDynamicBuffer
	{
		mu_uint32 Offset = 0u, Count = 0u;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> Buffer;
	};

	struct NImmutableBuffer
	{
		std::unique_ptr<NDynamicBuffer> VertexBuffer;
		std::unique_ptr<NDynamicBuffer> IndexBuffer;
		NGraphicsTexture *Texture = nullptr;
	};

	typedef std::unique_ptr<NDynamicBuffer> NDynamicBufferPtr;

    class NRenderInterface : public Rml::RenderInterface
    {
    public:
        NRenderInterface();
		~NRenderInterface();

		const mu_boolean Initialize();

        void BeginFrame();
		void EndFrame();
		void ReleaseGarbage();

	private:
		mu_boolean UpdateVertexBuffer();
		mu_boolean UpdateIndexBuffer();

		void DrawGeometry(
			Diligent::IBuffer *VertexBuffer,
			mu_uint32 verticesCount,
			mu_uint32 verticesOffset,
			Diligent::IBuffer *IndexBuffer,
			mu_uint32 indicesCount,
			mu_uint32 indicesOffset,
			NGraphicsTexture *Texture
		);

	public:
	    /// Called by RmlUi when it wants to render geometry that the application does not wish to optimise. Note that
	    /// RmlUi renders everything as triangles.
	    /// @param[in] vertices The geometry's vertex data.
	    /// @param[in] num_vertices The number of vertices passed to the function.
	    /// @param[in] indices The geometry's index data.
	    /// @param[in] num_indices The number of indices passed to the function. This will always be a multiple of three.
	    /// @param[in] texture The texture to be applied to the geometry. This may be nullptr, in which case the geometry is untextured.
	    /// @param[in] translation The translation to apply to the geometry.
        void RenderGeometry(
            Rml::Vertex* vertices,
            mu_int32 num_vertices,
            mu_int32* indices,
            mu_int32 num_indices,
            Rml::TextureHandle texture,
            const Rml::Vector2f& translation
        ) override;

		/// Called by RmlUi when it wants to compile geometry it believes will be static for the forseeable future.
		/// If supported, this should return a handle to an optimised, application-specific version of the data. If
		/// not, do not override the function or return zero; the simpler RenderGeometry() will be called instead.
		/// @param[in] vertices The geometry's vertex data.
		/// @param[in] num_vertices The number of vertices passed to the function.
		/// @param[in] indices The geometry's index data.
		/// @param[in] num_indices The number of indices passed to the function. This will always be a multiple of three.
		/// @param[in] texture The texture to be applied to the geometry. This may be nullptr, in which case the geometry is untextured.
		/// @return The application-specific compiled geometry. Compiled geometry will be stored and rendered using RenderCompiledGeometry() in future calls, and released with ReleaseCompiledGeometry() when it is no longer needed.
        Rml::CompiledGeometryHandle CompileGeometry(
            Rml::Vertex* vertices,
            mu_int32 num_vertices,
            mu_int32* indices,
            mu_int32 num_indices,
            Rml::TextureHandle texture
		) override;
		/// Called by RmlUi when it wants to render application-compiled geometry.
		/// @param[in] geometry The application-specific compiled geometry to render.
		/// @param[in] translation The translation to apply to the geometry.
		void RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, const Rml::Vector2f &translation) override;
		/// Called by RmlUi when it wants to release application-compiled geometry.
		/// @param[in] geometry The application-specific compiled geometry to release.
		void ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) override;

		/// Called by RmlUi when it wants to enable or disable scissoring to clip content.
		/// @param[in] enable True if scissoring is to enabled, false if it is to be disabled.
		void EnableScissorRegion(mu_boolean enable) override;
		/// Called by RmlUi when it wants to change the scissor region.
		/// @param[in] x The left-most pixel to be rendered. All pixels to the left of this should be clipped.
		/// @param[in] y The top-most pixel to be rendered. All pixels to the top of this should be clipped.
		/// @param[in] width The width of the scissored region. All pixels to the right of (x + width) should be clipped.
		/// @param[in] height The height of the scissored region. All pixels to below (y + height) should be clipped.
		void SetScissorRegion(mu_int32 x, mu_int32 y, mu_int32 width, mu_int32 height) override;

		/// Called by RmlUi when a texture is required by the library.
		/// @param[out] texture_handle The handle to write the texture handle for the loaded texture to.
		/// @param[out] texture_dimensions The variable to write the dimensions of the loaded texture.
		/// @param[in] source The application-defined image source, joined with the path of the referencing document.
		/// @return True if the load attempt succeeded and the handle and dimensions are valid, false if not.
		mu_boolean LoadTexture(Rml::TextureHandle &texture_handle, Rml::Vector2i &texture_dimensions, const Rml::String &source) override;
		/// Called by RmlUi when a texture is required to be built from an internally-generated sequence of pixels.
		/// @param[out] texture_handle The handle to write the texture handle for the generated texture to.
		/// @param[in] source The raw 8-bit texture data. Each pixel is made up of four 8-bit values, indicating red, green, blue and alpha in that order.
		/// @param[in] source_dimensions The dimensions, in pixels, of the source data.
		/// @return True if the texture generation succeeded and the handle is valid, false if not.
		mu_boolean GenerateTexture(Rml::TextureHandle &texture_handle, const Rml::byte *source, const Rml::Vector2i &source_dimensions) override;
		/// Called by RmlUi when a loaded texture is no longer required.
		/// @param texture The texture handle to release.
		void ReleaseTexture(Rml::TextureHandle texture) override;

		/// Called by RmlUi when it wants the renderer to use a new transform matrix.
		/// This will only be called if 'transform' properties are encountered. If no transform applies to the current element, nullptr
		/// is submitted. Then it expects the renderer to use an identity matrix or otherwise omit the multiplication with the transform.
		/// @param[in] transform The new transform to apply, or nullptr if no transform applies to the current element.
		void SetTransform(const Rml::Matrix4f *transform) override;

	private:
		mu_shader ColorShader = NInvalidShader;
		mu_shader TextureShader = NInvalidShader;
		glm::mat4 Projection;
		glm::mat4 Transform;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> RmlUniform;
		std::vector<Rml::Vertex> DynamicVertices;
		std::vector<mu_int32> DynamicIndices;
		NDynamicBufferPtr DynamicVertexBuffer;
		NDynamicBufferPtr DynamicIndexBuffer;
		std::vector<NDynamicBufferPtr> DynamicBuffersToRelease;

		mu_float Width, Height;
        SDL_Rect ScissorRect = {};
		mu_boolean UseTransform = false;
        mu_boolean UseScissor = false;
		mu_boolean RenderStencil = false;
		ScissorMode RequiredScissorMode = ScissorMode::None;
		ScissorMode CurrentScissorMode = ScissorMode::None;
    };
}

#endif