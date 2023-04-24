#ifndef __T_GRAPHICS_RENDERCLASSIFIER_H__
#define __T_GRAPHICS_RENDERCLASSIFIER_H__

#pragma once

enum class NRenderClassify : mu_uint32
{
	None,
	Opaque,
	PreAlpha,
	PostAlpha,
};

const NRenderClassify GetRenderClassify(
	const mu_boolean depthWrite,
	const mu_boolean blendEnable,
	const Diligent::BLEND_FACTOR srcBlend,
	const Diligent::BLEND_FACTOR dstBlend,
	const mu_uint32 blendHash
);

#endif