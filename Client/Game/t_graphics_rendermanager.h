#ifndef __T_GRAPHICS_RENDERMANAGER_H__
#define __T_GRAPHICS_RENDERMANAGER_H__

#pragma once

#include "t_graphics_renderclassifier.h"

enum class NRenderCommandType : mu_uint32
{
	UpdateBuffer,
	UpdateBufferWithMap,
	UpdateTexture,
	SetDynamicTexture,
	SetDynamicBuffer,
	SetPipelineState,
	SetVertexBuffer,
	SetIndexBuffer,
	CommitShaderResources,
	Draw,
	DrawIndexed,
};

struct RUpdateBuffer
{
	const mu_boolean ShouldReleaseMemory;
	Diligent::IBuffer *Buffer;
	void *Data;
	Diligent::Uint64 Offset;
	Diligent::Uint64 Size;
	Diligent::RESOURCE_STATE_TRANSITION_MODE StateTransitionMode;
};

struct RUpdateBufferWithMap
{
	const mu_boolean ShouldReleaseMemory;
	Diligent::IBuffer *Buffer;
	void *Data;
	Diligent::Uint64 Size;
	Diligent::MAP_TYPE MapType;
	Diligent::MAP_FLAGS MapFlags;
};

struct RUpdateTexture
{
	Diligent::ITexture *Texture;
	Diligent::Uint32 MipLevel;
	Diligent::Uint32 Slice;
	const Diligent::Box DstBox;
	const Diligent::TextureSubResData SubresData;
	Diligent::RESOURCE_STATE_TRANSITION_MODE SrcBufferTransitionMode;
	Diligent::RESOURCE_STATE_TRANSITION_MODE TextureTransitionMode;
};

struct RSetDynamicTexture
{
	const Diligent::SHADER_TYPE Type;
	const char *Name;
	Diligent::ITextureView *View;
	Diligent::IShaderResourceBinding *Binding;
};

struct RSetDynamicBuffer
{
	const Diligent::SHADER_TYPE Type;
	const char *Name;
	Diligent::IBuffer *Buffer;
	Diligent::IShaderResourceBinding *Binding;
};

struct NPipelineState;
struct RSetPipelineState
{
	NPipelineState *Pipeline;
};

struct RSetVertexBuffer
{
	Diligent::Uint32 StartSlot;
	Diligent::IBuffer *Buffer;
	const Diligent::Uint64 Offset;
	Diligent::RESOURCE_STATE_TRANSITION_MODE StateTransitionMode;
	Diligent::SET_VERTEX_BUFFERS_FLAGS Flags;
};

struct RSetIndexBuffer
{
	Diligent::IBuffer *IndexBuffer;
	Diligent::Uint64 ByteOffset;
	Diligent::RESOURCE_STATE_TRANSITION_MODE StateTransitionMode;
};

struct RCommitShaderResources
{
	NShaderResourcesBinding *ShaderResourceBinding;
	Diligent::RESOURCE_STATE_TRANSITION_MODE StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE;
};

struct RDraw
{
	Diligent::DrawAttribs Attribs;
};

struct RDrawIndexed
{
	Diligent::DrawIndexedAttribs Attribs;
};

struct NRenderCommand
{
	NRenderCommandType Type;
	union
	{
		RUpdateBuffer updateBuffer;
		RUpdateBufferWithMap updateBufferWithMap;
		RUpdateTexture updateTexture;
		RSetDynamicTexture setDynamicTexture;
		RSetDynamicBuffer setDynamicBuffer;
		RSetPipelineState setPipelineState;
		RSetVertexBuffer setVertexBuffer;
		RSetIndexBuffer setIndexBuffer;
		RCommitShaderResources commitShaderResources;
		RDraw draw;
		RDrawIndexed drawIndexed;
	};
};

enum class NDrawOrderType : mu_uint32
{
	Classifier,
	Sequential,
};

struct RCommandListInfo
{
	NDrawOrderType Type;
	NRenderClassify Classify;
	mu_uint16 View;
	union
	{
		mu_uint16 Index;
		mu_uint16 Depth;
	};
};

typedef mu_uint64 RCommandListHash;
class RCommandList
{
public:
	RCommandListHash Id;
	std::vector<NRenderCommand> Commands;
};

class NRenderManager
{
public:
	NRenderManager();

	void Execute(Diligent::IDeviceContext *immediateContext);

public:
	void UpdateBuffer(const RUpdateBuffer &data);
	void UpdateBufferWithMap(const RUpdateBufferWithMap &data);
	void UpdateTexture(const RUpdateTexture &data);
	void SetDynamicTexture(const RSetDynamicTexture &data);
	void SetDynamicBuffer(const RSetDynamicBuffer &data);
	void SetPipelineState(NPipelineState *pipeline);
	void SetVertexBuffer(const RSetVertexBuffer &data);
	void SetIndexBuffer(const RSetIndexBuffer &data);
	void CommitShaderResources(const RCommitShaderResources &data);
	void Draw(const RDraw &data, const RCommandListInfo &info);
	void DrawIndexed(const RDrawIndexed &data, const RCommandListInfo &info);

private:
	void PushCommandList(const RCommandListInfo &info);

private:
	mu_uint32 Index = 0;
	NPipelineStateInfo *PipelineInfo = nullptr;
	RCommandList *DraftCommandList = nullptr;
	std::vector<RCommandList> CommandLists;
};

#endif