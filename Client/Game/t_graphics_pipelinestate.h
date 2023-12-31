#ifndef __T_GRAPHICS_PIPELINESTATE_H__
#define __T_GRAPHICS_PIPELINESTATE_H__

#pragma once

constexpr mu_uint32 EnableScissorsBits = 1;
constexpr mu_uint32 ColorWriteBits = 1;
constexpr mu_uint32 AlphaWriteBits = 1;
constexpr mu_uint32 CullModeBits = ComputeBitsNeeded(Diligent::CULL_MODE_NUM_MODES); // no extra bit since we force it to be uint8
constexpr mu_uint32 DepthWriteBits = 1;
constexpr mu_uint32 DepthFuncBits = ComputeBitsNeeded(Diligent::COMPARISON_FUNC_NUM_FUNCTIONS);
constexpr mu_uint32 StencilEnableBits = 1;
constexpr mu_uint32 StencilMaskBits = sizeof(mu_uint8) * 8;
constexpr mu_uint32 StencilOperationBits = ComputeBitsNeeded(Diligent::STENCIL_OP_NUM_OPS); // no extra bit since we force it to be uint8
constexpr mu_uint32 StencilFuncBits = ComputeBitsNeeded(Diligent::COMPARISON_FUNC_NUM_FUNCTIONS);
constexpr mu_uint32 BlendFactorBits = ComputeBitsNeeded(Diligent::BLEND_FACTOR_NUM_FACTORS); // no extra bit since we force it to be uint8 (limited because there is some factors we don't require)
constexpr mu_uint32 BlendOperationBits = ComputeBitsNeeded(Diligent::BLEND_OPERATION_NUM_OPERATIONS); // no extra bit since we force it to be uint8

constexpr mu_uint32 DynamicPipelineBits = (
	CullModeBits +
	EnableScissorsBits +
	ColorWriteBits +
	AlphaWriteBits +
	DepthWriteBits +
	DepthFuncBits +
	StencilEnableBits +
	StencilMaskBits +
	//StencilMaskBits +
	StencilOperationBits +
	StencilOperationBits +
	StencilOperationBits +
	StencilFuncBits +
	BlendFactorBits +
	BlendFactorBits +
	BlendFactorBits +
	BlendFactorBits +
	BlendOperationBits
);

constexpr mu_uint64 CullModeShift = 0;
constexpr mu_uint64 EnableScissorsShift = CullModeShift + CullModeBits;
constexpr mu_uint64 ColorWriteShift = EnableScissorsShift + EnableScissorsBits;
constexpr mu_uint64 AlphaWriteShift = ColorWriteShift + ColorWriteBits;
constexpr mu_uint64 DepthWriteShift = AlphaWriteShift + AlphaWriteBits;
constexpr mu_uint64 DepthFuncShift = DepthWriteShift + DepthWriteBits;
constexpr mu_uint64 StencilEnableShift = DepthFuncShift + DepthFuncBits;
constexpr mu_uint64 StencilReadMaskShift = StencilEnableShift + StencilEnableBits;
//constexpr mu_uint64 StencilWriteMaskShift = StencilReadMaskShift + StencilMaskBits;
constexpr mu_uint64 StencilFailOpShift = StencilReadMaskShift + StencilMaskBits;
constexpr mu_uint64 StencilDepthFailOpShift = StencilFailOpShift + StencilOperationBits;
constexpr mu_uint64 StencilPassOpShift = StencilDepthFailOpShift + StencilOperationBits;
constexpr mu_uint64 StencilFuncShift = StencilPassOpShift + StencilOperationBits;
constexpr mu_uint64 SrcBlendShift = StencilFuncShift + StencilFuncBits;
constexpr mu_uint64 DestBlendShift = SrcBlendShift + BlendFactorBits;
constexpr mu_uint64 SrcBlendAlphaShift = DestBlendShift + BlendFactorBits;
constexpr mu_uint64 DestBlendAlphaShift = SrcBlendAlphaShift + BlendFactorBits;
constexpr mu_uint64 BlendOpShift = DestBlendAlphaShift + BlendFactorBits;
constexpr mu_uint64 BlendOpAlphaShift = BlendOpShift + BlendOperationBits;

typedef mu_uint64 NDynamicPipelineHash;
struct NDynamicPipelineState
{
	Diligent::Uint8 CullMode : CullModeBits = Diligent::CULL_MODE_NONE;

	Diligent::Bool EnableScissors : EnableScissorsBits = false;
	Diligent::Bool ColorWrite : ColorWriteBits = true;
	Diligent::Bool AlphaWrite : AlphaWriteBits = true;
	Diligent::Bool DepthWrite : DepthWriteBits = true;
	Diligent::COMPARISON_FUNCTION DepthFunc : DepthFuncBits = Diligent::COMPARISON_FUNC_LESS;

	Diligent::Bool StencilEnable : StencilEnableBits = false;
	Diligent::Uint8 StencilReadMask : StencilMaskBits = 0xFF;
	//Diligent::Uint8 StencilWriteMask : StencilMaskBits = 0xFF;
	Diligent::Uint8 StencilFailOp : StencilOperationBits = Diligent::STENCIL_OP_KEEP;
	Diligent::Uint8 StencilDepthFailOp : StencilOperationBits = Diligent::STENCIL_OP_KEEP;
	Diligent::Uint8 StencilPassOp : StencilOperationBits = Diligent::STENCIL_OP_KEEP;
	Diligent::COMPARISON_FUNCTION StencilFunc : StencilFuncBits = Diligent::COMPARISON_FUNC_ALWAYS;

	Diligent::Uint8 SrcBlend : BlendFactorBits = Diligent::BLEND_FACTOR_UNDEFINED; // Diligent::BLEND_FACTOR is Int8, which forces us to have one extra bit for the sign
	Diligent::Uint8 DestBlend : BlendFactorBits = Diligent::BLEND_FACTOR_UNDEFINED;
	Diligent::Uint8 SrcBlendAlpha : BlendFactorBits = Diligent::BLEND_FACTOR_UNDEFINED;
	Diligent::Uint8 DestBlendAlpha : BlendFactorBits = Diligent::BLEND_FACTOR_UNDEFINED;
	Diligent::Uint8 BlendOp : BlendOperationBits = Diligent::BLEND_OPERATION_ADD;
	Diligent::Uint8 BlendOpAlpha : BlendOperationBits = Diligent::BLEND_OPERATION_ADD;

	Diligent::CULL_MODE GetCullMode() const
	{
		return static_cast<Diligent::CULL_MODE>(CullMode);
	}

	Diligent::STENCIL_OP GetStencilFailOp() const
	{
		return static_cast<Diligent::STENCIL_OP>(StencilFailOp);
	}

	Diligent::STENCIL_OP GetStencilDepthFailOp() const
	{
		return static_cast<Diligent::STENCIL_OP>(StencilDepthFailOp);
	}

	Diligent::STENCIL_OP GetStencilPassOp() const
	{
		return static_cast<Diligent::STENCIL_OP>(StencilPassOp);
	}

	Diligent::COMPARISON_FUNCTION GetStencilFunc() const
	{
		return static_cast<Diligent::COMPARISON_FUNCTION>(StencilFunc);
	}

	Diligent::BLEND_FACTOR GetSrcBlend() const
	{
		return static_cast<Diligent::BLEND_FACTOR>(SrcBlend);
	}

	Diligent::BLEND_FACTOR GetDestBlend() const
	{
		return static_cast<Diligent::BLEND_FACTOR>(DestBlend);
	}

	Diligent::BLEND_FACTOR GetSrcBlendAlpha() const
	{
		return static_cast<Diligent::BLEND_FACTOR>(SrcBlendAlpha);
	}

	Diligent::BLEND_FACTOR GetDestBlendAlpha() const
	{
		return static_cast<Diligent::BLEND_FACTOR>(DestBlendAlpha);
	}

	Diligent::BLEND_OPERATION GetBlendOp() const
	{
		return static_cast<Diligent::BLEND_OPERATION>(BlendOp);
	}

	Diligent::BLEND_OPERATION GetBlendOpAlpha() const
	{
		return static_cast<Diligent::BLEND_OPERATION>(BlendOpAlpha);
	}

	const NDynamicPipelineHash GetHash() const
	{
		return (
			(static_cast<mu_uint64>(CullMode) << CullModeShift) |
			(static_cast<mu_uint64>(EnableScissors) << EnableScissorsShift) |
			(static_cast<mu_uint64>(ColorWrite) << ColorWriteShift) |
			(static_cast<mu_uint64>(AlphaWrite) << AlphaWriteShift) |
			(static_cast<mu_uint64>(DepthWrite) << DepthWriteShift) |
			(static_cast<mu_uint64>(DepthFunc) << DepthFuncShift) |
			(static_cast<mu_uint64>(StencilEnable) << StencilEnableShift) |
			(static_cast<mu_uint64>(StencilReadMask) << StencilReadMaskShift) |
			//(static_cast<mu_uint64>(StencilWriteMask) << StencilWriteMaskShift) |
			(static_cast<mu_uint64>(StencilFailOp) << StencilFailOpShift) |
			(static_cast<mu_uint64>(StencilDepthFailOp) << StencilDepthFailOpShift) |
			(static_cast<mu_uint64>(StencilPassOp) << StencilPassOpShift) |
			(static_cast<mu_uint64>(StencilFunc) << StencilFuncShift) |
			(static_cast<mu_uint64>(SrcBlend) << SrcBlendShift) |
			(static_cast<mu_uint64>(DestBlend) << DestBlendShift) |
			(static_cast<mu_uint64>(SrcBlendAlpha) << SrcBlendAlphaShift) |
			(static_cast<mu_uint64>(DestBlendAlpha) << DestBlendAlphaShift) |
			(static_cast<mu_uint64>(BlendOp) << BlendOpShift) |
			(static_cast<mu_uint64>(BlendOpAlpha) << BlendOpAlphaShift)
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
extern NDynamicPipelineState DefaultShadowDynamicPipelineState;

NEXTMU_INLINE NDynamicPipelineState NormalizeShadowRenderState(NDynamicPipelineState renderState)
{
	renderState.CullMode = Diligent::CULL_MODE_NONE;
	renderState.ColorWrite = false;
	renderState.AlphaWrite = false;

	renderState.StencilEnable = false;
	renderState.StencilReadMask = 0xFF;
	//renderState.StencilWriteMask = 0xFF;
	renderState.StencilFailOp = Diligent::STENCIL_OP_KEEP;
	renderState.StencilDepthFailOp = Diligent::STENCIL_OP_KEEP;
	renderState.StencilPassOp = Diligent::STENCIL_OP_KEEP;
	renderState.StencilFunc = Diligent::COMPARISON_FUNC_ALWAYS;

	renderState.SrcBlend = Diligent::BLEND_FACTOR_UNDEFINED;
	renderState.DestBlend = Diligent::BLEND_FACTOR_UNDEFINED;
	renderState.SrcBlendAlpha = Diligent::BLEND_FACTOR_UNDEFINED;
	renderState.DestBlendAlpha = Diligent::BLEND_FACTOR_UNDEFINED;
	renderState.BlendOp = Diligent::BLEND_OPERATION_ADD;
	renderState.BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;

	return renderState;
}

#endif