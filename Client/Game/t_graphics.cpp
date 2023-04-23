#include "stdafx.h"
#include "t_graphics.h"

NDynamicPipelineState DefaultDynamicPipelineState;
NDynamicPipelineState DefaultAlphaDynamicPipelineState{
	.SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA,
	.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
};