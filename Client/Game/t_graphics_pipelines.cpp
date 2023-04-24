#include "stdafx.h"
#include "t_graphics_pipelines.h"
#include "mu_graphics.h"

typedef std::map<NDynamicPipelineHash, NPipelineState> DynamicPipelineMap;
typedef std::map<NFixedPipelineHash, DynamicPipelineMap> FixedPipelineMap;
FixedPipelineMap Pipelines;
mu_atomic_uint32_t PipelineIdCache(0u);

void DestroyPipelines()
{
    Pipelines.clear();
}

const NPipelineBlendHash CalculateBlendHash(
    const NPipelineBlendHash src,
    const NPipelineBlendHash dst
)
{
    return "\x0\x2\x2\x3\x3\x2\x3\x2\x3\x2\x2\x2\x2\x2\x2\x2\x2\x2\x2"[src + (!!src)];
    /*constexpr NPipelineBlendHash BlendBits = ComputeBitsNeeded(Diligent::BLEND_FACTOR_NUM_FACTORS);
    return (
        (dst << BlendBits) |
        src
    );*/
}

NPipelineState *CreatePipelineState(const NFixedPipelineState &fixedState, const NDynamicPipelineState &dynamicState)
{
    auto shader = GetShader(fixedState.CombinedShader);

    Diligent::GraphicsPipelineStateCreateInfo createInfo;
    auto &graphicsInfo = createInfo.GraphicsPipeline;
    auto &pipelineStateDesc = createInfo.PSODesc;

    // Configure Rasterizer
    {
        auto &rasterizer = graphicsInfo.RasterizerDesc;
        rasterizer.CullMode = dynamicState.CullMode;
    }

    // Configure Blend
    {
        auto &blend = graphicsInfo.BlendDesc.RenderTargets[0];
        blend.BlendEnable = (
            dynamicState.SrcBlend != Diligent::BLEND_FACTOR_UNDEFINED &&
            dynamicState.DestBlend != Diligent::BLEND_FACTOR_UNDEFINED &&
            dynamicState.BlendOp != Diligent::BLEND_OPERATION_UNDEFINED
            );
        if (blend.BlendEnable)
        {
            blend.SrcBlend = dynamicState.SrcBlend;
            blend.DestBlend = dynamicState.DestBlend;
            blend.BlendOp = dynamicState.BlendOp;
            blend.SrcBlendAlpha = dynamicState.SrcBlend;
            blend.DestBlendAlpha = dynamicState.DestBlend;
            blend.BlendOpAlpha = dynamicState.BlendOp;
        }

        blend.RenderTargetWriteMask = Diligent::COLOR_MASK_NONE;
        if (dynamicState.ColorWrite)
            blend.RenderTargetWriteMask |= Diligent::COLOR_MASK_RED | Diligent::COLOR_MASK_GREEN | Diligent::COLOR_MASK_BLUE;
        if (dynamicState.AlphaWrite)
            blend.RenderTargetWriteMask |= Diligent::COLOR_MASK_ALPHA;
    }

    // Configure Depth Stencil
    {
        auto &depthStencil = graphicsInfo.DepthStencilDesc;

        depthStencil.DepthWriteEnable = dynamicState.DepthWrite;
        depthStencil.DepthEnable = dynamicState.DepthFunc != Diligent::COMPARISON_FUNC_UNKNOWN;
        depthStencil.DepthFunc = dynamicState.DepthFunc;

        depthStencil.StencilEnable = dynamicState.StencilEnable;
        depthStencil.StencilReadMask = dynamicState.StencilReadMask;
        depthStencil.StencilWriteMask = dynamicState.StencilWriteMask;

        auto &stencilFront = depthStencil.FrontFace;
        stencilFront.StencilFailOp = dynamicState.StencilFailOp;
        stencilFront.StencilDepthFailOp = dynamicState.StencilDepthFailOp;
        stencilFront.StencilPassOp = dynamicState.StencilPassOp;
        stencilFront.StencilFunc = dynamicState.StencilFunc;

        auto &stencilBack = depthStencil.BackFace;
        stencilBack.StencilFailOp = dynamicState.StencilFailOp;
        stencilBack.StencilDepthFailOp = dynamicState.StencilDepthFailOp;
        stencilBack.StencilPassOp = dynamicState.StencilPassOp;
        stencilBack.StencilFunc = dynamicState.StencilFunc;
    }

    // Configure Shader
    {
        graphicsInfo.InputLayout = shader->Layout;
        createInfo.pVS = shader->Vertex.RawPtr();
        createInfo.pPS = shader->Pixel.RawPtr();

        const auto resource = shader->Resource;
        auto &resourceLayout = pipelineStateDesc.ResourceLayout;
        resourceLayout.NumVariables = static_cast<mu_uint32>(resource->Variables.size());
        resourceLayout.Variables = resource->Variables.data();
        resourceLayout.NumImmutableSamplers = static_cast<mu_uint32>(resource->ImmutableSamplers.size());
        resourceLayout.ImmutableSamplers = resource->ImmutableSamplers.data();
    }

    graphicsInfo.NumRenderTargets = 1;
    graphicsInfo.RTVFormats[0] = fixedState.RTVFormat;
    graphicsInfo.DSVFormat = fixedState.DSVFormat;

    const auto device = MUGraphics::GetDevice();
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipeline;
    device->CreateGraphicsPipelineState(createInfo, &pipeline);
    if (pipeline == nullptr)
    {
        return nullptr;
    }

    const auto &blendDesc = graphicsInfo.BlendDesc.RenderTargets[0];
    NPipelineState pipelineState;
    pipelineState.Id = PipelineIdCache++;
    pipelineState.Info.Shader = fixedState.CombinedShader;
    pipelineState.Info.BlendEnable = blendDesc.BlendEnable;
    pipelineState.Info.SrcBlend = blendDesc.SrcBlend;
    pipelineState.Info.DestBlend = blendDesc.DestBlend;
    pipelineState.Info.BlendHash = (
        blendDesc.BlendEnable
        ? CalculateBlendHash(blendDesc.SrcBlend, blendDesc.DestBlend)
        : 0
    );
    pipelineState.Pipeline = pipeline;

    auto fixedIter = Pipelines.find(fixedState.GetHash());
    if (fixedIter == Pipelines.end())
    {
        fixedIter = Pipelines.insert(std::make_pair(fixedState.GetHash(), DynamicPipelineMap())).first;
    }

    auto &dynamicMap = fixedIter->second;
    auto dynamicIter = dynamicMap.insert(std::make_pair(dynamicState.GetHash(), pipelineState)).first;

    return &dynamicIter->second;
}

NPipelineState *GetPipelineState(const NFixedPipelineState &fixedState, const NDynamicPipelineState &dynamicState)
{
    auto fixedIter = Pipelines.find(fixedState.GetHash());
    if (fixedIter == Pipelines.end()) return CreatePipelineState(fixedState, dynamicState);
    auto &pipelines = fixedIter->second;
    auto dynamicIter = pipelines.find(dynamicState.GetHash());
    if (dynamicIter == pipelines.end()) return CreatePipelineState(fixedState, dynamicState);
    return &dynamicIter->second;
}