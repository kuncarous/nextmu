#ifndef __T_GRAPHICS_H__
#define __T_GRAPHICS_H__

#pragma once

#include "t_graphics_layouts.h"
#include "t_graphics_shader.h"
#include "t_graphics_resources.h"
#include "t_graphics_pipelines.h"
#include "t_graphics_samplers.h"
#include "t_graphics_rendermanager.h"

constexpr mu_uint32 ColorWriteBits = 1;
constexpr mu_uint32 AlphaWriteBits = 1;
constexpr mu_uint32 CullModeBits = ComputeBitsNeeded(Diligent::CULL_MODE_NUM_MODES) + 1; // 1 extra byte for the sign (-/+)
constexpr mu_uint32 DepthWriteBits = 1;
constexpr mu_uint32 DepthFuncBits = ComputeBitsNeeded(Diligent::COMPARISON_FUNC_NUM_FUNCTIONS);
constexpr mu_uint32 StencilEnableBits = 1;
constexpr mu_uint32 StencilMaskBits = sizeof(mu_uint8) * 8;
constexpr mu_uint32 StencilOperationBits = ComputeBitsNeeded(Diligent::STENCIL_OP_NUM_OPS) + 1; // 1 extra byte for the sign (-/+)
constexpr mu_uint32 StencilFuncBits = ComputeBitsNeeded(Diligent::COMPARISON_FUNC_NUM_FUNCTIONS);
constexpr mu_uint32 BlendFactorBits = ComputeBitsNeeded(Diligent::BLEND_FACTOR_NUM_FACTORS) + 1; // 1 extra byte for the sign (-/+)
constexpr mu_uint32 BlendOperationBits = ComputeBitsNeeded(Diligent::BLEND_OPERATION_NUM_OPERATIONS) + 1; // 1 extra byte for the sign (-/+)

constexpr mu_uint32 DynamicPipelineBits = (
	CullModeBits +
	ColorWriteBits +
	AlphaWriteBits +
	DepthWriteBits +
	DepthFuncBits +
	StencilEnableBits +
	StencilMaskBits +
	StencilMaskBits +
	StencilOperationBits +
	StencilOperationBits +
	StencilOperationBits +
	StencilFuncBits +
	BlendFactorBits +
	BlendFactorBits +
	BlendOperationBits
);

constexpr mu_uint64 CullModeShift = 0;
constexpr mu_uint64 ColorWriteShift = CullModeShift + CullModeBits;
constexpr mu_uint64 AlphaWriteShift = ColorWriteShift + ColorWriteBits;
constexpr mu_uint64 DepthWriteShift = AlphaWriteShift + AlphaWriteBits;
constexpr mu_uint64 DepthFuncShift = DepthWriteShift + DepthWriteBits;
constexpr mu_uint64 StencilEnableShift = DepthFuncShift + DepthFuncBits;
constexpr mu_uint64 StencilReadMaskShift = StencilEnableShift + StencilEnableBits;
constexpr mu_uint64 StencilWriteMaskShift = StencilReadMaskShift + StencilMaskBits;
constexpr mu_uint64 StencilFailOpShift = StencilWriteMaskShift + StencilMaskBits;
constexpr mu_uint64 StencilDepthFailOpShift = StencilFailOpShift + StencilOperationBits;
constexpr mu_uint64 StencilPassOpShift = StencilDepthFailOpShift + StencilOperationBits;
constexpr mu_uint64 StencilFuncShift = StencilPassOpShift + StencilOperationBits;
constexpr mu_uint64 SrcBlendShift = StencilFuncShift + StencilFuncBits;
constexpr mu_uint64 DestBlendShift = SrcBlendShift + BlendFactorBits;
constexpr mu_uint64 BlendOpShift = DestBlendShift + BlendFactorBits;

typedef mu_uint64 NDynamicPipelineHash;
struct NDynamicPipelineState
{
	Diligent::CULL_MODE CullMode : CullModeBits = Diligent::CULL_MODE_NONE;

	Diligent::Bool ColorWrite : ColorWriteBits = true;
	Diligent::Bool AlphaWrite : AlphaWriteBits = true;
	Diligent::Bool DepthWrite : DepthWriteBits = true;
	Diligent::COMPARISON_FUNCTION DepthFunc : DepthFuncBits = Diligent::COMPARISON_FUNC_LESS;

	Diligent::Bool StencilEnable : StencilEnableBits = false;
	Diligent::Uint8 StencilReadMask : StencilMaskBits = 0xFF;
	Diligent::Uint8 StencilWriteMask : StencilMaskBits = 0xFF;
	Diligent::STENCIL_OP StencilFailOp : StencilOperationBits = Diligent::STENCIL_OP_KEEP;
	Diligent::STENCIL_OP StencilDepthFailOp : StencilOperationBits = Diligent::STENCIL_OP_KEEP;
	Diligent::STENCIL_OP StencilPassOp : StencilOperationBits = Diligent::STENCIL_OP_KEEP;
	Diligent::COMPARISON_FUNCTION StencilFunc : StencilFuncBits = Diligent::COMPARISON_FUNC_ALWAYS;

	Diligent::BLEND_FACTOR SrcBlend : BlendFactorBits = Diligent::BLEND_FACTOR_UNDEFINED;
	Diligent::BLEND_FACTOR DestBlend : BlendFactorBits = Diligent::BLEND_FACTOR_UNDEFINED;
	Diligent::BLEND_OPERATION BlendOp : BlendOperationBits = Diligent::BLEND_OPERATION_ADD;

	const NDynamicPipelineHash GetHash() const
	{
		return (
			(static_cast<mu_uint64>(CullMode) << CullModeShift) |
			(static_cast<mu_uint64>(ColorWrite) << ColorWriteShift) |
			(static_cast<mu_uint64>(AlphaWrite) << AlphaWriteShift) |
			(static_cast<mu_uint64>(DepthWrite) << DepthWriteShift) |
			(static_cast<mu_uint64>(DepthFunc) << DepthFuncShift) |
			(static_cast<mu_uint64>(StencilEnable) << StencilEnableShift) |
			(static_cast<mu_uint64>(StencilReadMask) << StencilReadMaskShift) |
			(static_cast<mu_uint64>(StencilWriteMask) << StencilWriteMaskShift) |
			(static_cast<mu_uint64>(StencilFailOp) << StencilFailOpShift) |
			(static_cast<mu_uint64>(StencilDepthFailOp) << StencilDepthFailOpShift) |
			(static_cast<mu_uint64>(StencilPassOp) << StencilPassOpShift) |
			(static_cast<mu_uint64>(StencilFunc) << StencilFuncShift) |
			(static_cast<mu_uint64>(SrcBlend) << SrcBlendShift) |
			(static_cast<mu_uint64>(DestBlend) << DestBlendShift) |
			(static_cast<mu_uint64>(BlendOp) << BlendOpShift)
		);
	}
};

constexpr mu_uint32 ShaderBits = sizeof(mu_shader) * 8;
constexpr mu_uint32 TextureFormatBits = ComputeBitsNeeded(Diligent::TEX_FORMAT_NUM_FORMATS);

constexpr mu_uint32 FixedPipelineBits = (
	ShaderBits +
	TextureFormatBits +
	TextureFormatBits
);

constexpr mu_uint64 CombinedShaderShift = 0;
constexpr mu_uint64 RTVFormatShift = CombinedShaderShift + ShaderBits;
constexpr mu_uint64 DSVFormatShift = RTVFormatShift + TextureFormatBits;

typedef mu_uint32 NFixedPipelineHash;
struct NFixedPipelineState
{
	mu_shader CombinedShader : ShaderBits;
	Diligent::TEXTURE_FORMAT RTVFormat : TextureFormatBits;
	Diligent::TEXTURE_FORMAT DSVFormat : TextureFormatBits;

	const NFixedPipelineHash GetHash() const
	{
		return (
			(static_cast<mu_uint64>(CombinedShader) << CombinedShaderShift) |
			(static_cast<mu_uint64>(RTVFormat) << RTVFormatShift) |
			(static_cast<mu_uint64>(DSVFormat) << DSVFormatShift)
		);
	}
};

extern NDynamicPipelineState DefaultDynamicPipelineState;
extern NDynamicPipelineState DefaultAlphaDynamicPipelineState;

#endif