#include "stdafx.h"
#include "t_graphics_rendermanager.h"
#include <MapHelper.hpp>

void NRenderManager::Execute(Diligent::IDeviceContext *immediateContext)
{
	std::sort(
		CommandLists.begin(),
		CommandLists.end(),
		[](const std::unique_ptr<RCommandList> &lhs, const std::unique_ptr<RCommandList> &rhs) -> bool { return lhs->Id < rhs->Id; }
	);

	for (auto &commandList : CommandLists)
	{
		for (auto &command : commandList->Commands)
		{
			switch (command.Type)
			{
			case NRenderCommandType::UpdateBuffer:
				{
					auto &data = command.updateBuffer;
					immediateContext->UpdateBuffer(data.Buffer, data.Offset, data.Size, data.Data, data.StateTransitionMode);
				}
				break;

			case NRenderCommandType::UpdateBufferWithMap:
				{
					auto &data = command.updateBufferWithMap;
					Diligent::MapHelper<void> mapped(immediateContext, data.Buffer, data.MapType, data.MapFlags);
					mu_memcpy(mapped, data.Data, data.Size);
				}
				break;

			case NRenderCommandType::UpdateTexture:
				{
					auto &data = command.updateTexture;
					immediateContext->UpdateTexture(data.Texture, data.MipLevel, data.Slice, data.DstBox, data.SubresData, data.SrcBufferTransitionMode, data.TextureTransitionMode);
				}
				break;

			case NRenderCommandType::SetDynamicTexture:
				{
					auto &data = command.setDynamicTexture;
					data.Binding->GetVariableByName(data.Type, data.Name)->Set(data.View);
				}
				break;

			case NRenderCommandType::SetDynamicBuffer:
				{
					auto &data = command.setDynamicBuffer;
					data.Binding->GetVariableByName(data.Type, data.Name)->Set(data.Buffer);
				}
				break;

			case NRenderCommandType::SetPipelineState:
				{
					auto &data = command.setPipelineState;
					immediateContext->SetPipelineState(data.Pipeline->Pipeline.RawPtr());
				}
				break;

			case NRenderCommandType::SetVertexBuffer:
				{
					auto &data = command.setVertexBuffer;
					immediateContext->SetVertexBuffers(data.StartSlot, 1, &data.Buffer, &data.Offset, data.StateTransitionMode, data.Flags);
				}
				break;

			case NRenderCommandType::SetIndexBuffer:
				{
					auto &data = command.setIndexBuffer;
					immediateContext->SetIndexBuffer(data.IndexBuffer, data.ByteOffset, data.StateTransitionMode);
				}
				break;

			case NRenderCommandType::CommitShaderResources:
				{
					auto &data = command.commitShaderResources;
					immediateContext->CommitShaderResources(data.ShaderResourceBinding, data.StateTransitionMode);
				}
				break;

			case NRenderCommandType::Draw:
				{
					auto &data = command.draw;
					immediateContext->Draw(data.Attribs);
				}
				break;

			case NRenderCommandType::DrawIndexed:
				{
					auto &data = command.drawIndexed;
					immediateContext->DrawIndexed(data.Attribs);
				}
				break;
			}
		}
	}

	CommandLists.clear();
}

void NRenderManager::UpdateBuffer(const RUpdateBuffer &data, std::unique_ptr<RTemporaryBuffer> tmpBuffer)
{
	auto &commandList = DraftCommandList;
	commandList.Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::UpdateBuffer,
			.TmpBuffer = std::move(tmpBuffer),
			.updateBuffer = data,
		}
	);
}

void NRenderManager::UpdateBufferWithMap(const RUpdateBufferWithMap &data, std::unique_ptr<RTemporaryBuffer> tmpBuffer)
{
	auto &commandList = DraftCommandList;
	commandList.Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::UpdateBufferWithMap,
			.TmpBuffer = std::move(tmpBuffer),
			.updateBufferWithMap = data,
		}
	);
}

void NRenderManager::UpdateTexture(const RUpdateTexture &data)
{
	auto &commandList = DraftCommandList;
	commandList.Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::UpdateTexture,
			.updateTexture = data,
		}
	);
}

void NRenderManager::SetDynamicTexture(const RSetDynamicTexture &data)
{
	auto &commandList = DraftCommandList;
	commandList.Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::SetDynamicTexture,
			.setDynamicTexture = data,
		}
	);
}

void NRenderManager::SetDynamicBuffer(const RSetDynamicBuffer &data)
{
	auto &commandList = DraftCommandList;
	commandList.Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::SetDynamicBuffer,
			.setDynamicBuffer = data,
		}
	);
}

void NRenderManager::SetPipelineState(NPipelineState *pipeline)
{
	auto &commandList = DraftCommandList;
	commandList.Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::SetPipelineState,
			.setPipelineState = RSetPipelineState{
				.Pipeline = pipeline,
			},
		}
	);
	commandList.Blend = pipeline->BlendHash;
}

void NRenderManager::SetVertexBuffer(const RSetVertexBuffer &data)
{
	auto &commandList = DraftCommandList;
	commandList.Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::SetVertexBuffer,
			.setVertexBuffer = data,
		}
	);
}

void NRenderManager::SetIndexBuffer(const RSetIndexBuffer &data)
{
	auto &commandList = DraftCommandList;
	commandList.Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::SetIndexBuffer,
			.setIndexBuffer = data,
		}
	);
}

void NRenderManager::CommitShaderResources(const RCommitShaderResources &data)
{
	auto &commandList = DraftCommandList;
	commandList.Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::CommitShaderResources,
			.commitShaderResources = data,
		}
	);
}

void NRenderManager::Draw(const RDraw &data, const RCommandListInfo &info)
{
	auto &commandList = DraftCommandList;
	commandList.Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::Draw,
			.draw = data,
		}
	);
	PushCommandList(info);
}

void NRenderManager::DrawIndexed(const RDrawIndexed &data, const RCommandListInfo &info)
{
	auto &commandList = DraftCommandList;
	commandList.Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::DrawIndexed,
			.drawIndexed = data,
		}
	);
	PushCommandList(info);
}

const mu_uint64 GetCommandListHash(
	const mu_uint64 view,
	const mu_uint64 blend,
	const mu_uint64 group,
	const mu_uint32 type,
	const mu_uint32 index
)
{
	constexpr mu_uint64 ViewMask = 0xFFFF;
	constexpr mu_uint64 GroupMask = 0xFF;
	constexpr mu_uint64 BlendMask = 0x3FF;
	constexpr mu_uint64 TypeMask = 0x7;
	constexpr mu_uint64 IndexMask = 0xFFFF;

	return ((view & ViewMask) << 48) | ((group & GroupMask) << 40) | ((blend & BlendMask) << 30) | ((type & TypeMask) << 27) | (index & IndexMask);
}

void NRenderManager::PushCommandList(const RCommandListInfo &info)
{
	DraftCommandList.Id = GetCommandListHash(
		info.View,
		0,
		DraftCommandList.Blend,
		static_cast<mu_uint32>(info.Type),
		info.Index
	);
	CommandLists.push_back(std::make_unique<RCommandList>(std::move(DraftCommandList)));
}