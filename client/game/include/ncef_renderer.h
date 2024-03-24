#ifndef __NCEF_RENDERER_H__
#define __NCEF_RENDERER_H__

#pragma once

#if NEXTMU_EMBEDDED_BROWSER == 1
#include <include/cef_render_handler.h>
#include "t_graphics_texture.h"

class NBrowserRenderer : public CefRenderHandler
{
public:
    ~NBrowserRenderer();
    const mu_boolean Initialize();
    void Render();
    void OnResize(mu_int32 w, mu_int32 h);
    void ReloadShaders();

public:
    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override;
    virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, mu_int32 w, mu_int32 h) override;
    
private:
    mu_boolean IsLinear = false;
    mu_int32 Width = -1, Height = -1;
    mu_uint32 TextureId = NInvalidUInt32;

    Diligent::RESOURCE_STATE TextureState = Diligent::RESOURCE_STATE_UNKNOWN;
	NGraphicsTexturePtr Texture;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> Uniform;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> VertexBuffer;

	mu_shader Shader = NInvalidShader;
	glm::mat4 Projection = glm::mat4(1.0f);
	glm::mat4 Transform = glm::mat4(1.0f);

	IMPLEMENT_REFCOUNTING(NBrowserRenderer);
};
#endif

#endif