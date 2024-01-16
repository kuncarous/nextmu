#ifndef __T_GRAPHICS_H__
#define __T_GRAPHICS_H__

#pragma once

#include "t_graphics_layouts.h"
#include "t_graphics_shader.h"
#include "t_graphics_pipelineresources.h"
#include "t_graphics_pipelines.h"
#include "t_graphics_samplers.h"
#include "t_graphics_texture.h"
#include "t_graphics_buffer.h"
#include "t_graphics_shaderresources.h"
#include "t_graphics_rendermanager.h"
#include "t_graphics_rendersettings.h"
#include "t_graphics_shadows.h"

struct NRenderTargetDesc
{
	Diligent::TEXTURE_FORMAT ColorFormat;
	Diligent::TEXTURE_FORMAT DepthStencilFormat;
};

#endif