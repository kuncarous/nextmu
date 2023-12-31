#include "stdafx.h"
#include "t_graphics_renderclassifier.h"

const NRenderClassify GetRenderClassify(
	const mu_boolean depthWrite,
	const mu_boolean blendEnable,
	const Diligent::BLEND_FACTOR srcBlend,
	const Diligent::BLEND_FACTOR dstBlend,
	const mu_uint32 blendHash
)
{
	if (blendEnable == false) return NRenderClassify::Opaque;
	//return !depthWrite ? NRenderClassify::PreAlpha : NRenderClassify::PostAlpha;
	return static_cast<NRenderClassify>("\x0\x2\x2\x3\x3\x2\x3\x2\x3\x2\x2\x2\x2\x2\x2\x2\x2\x2\x2"[((blendHash) & 0xf) + 1]);
}