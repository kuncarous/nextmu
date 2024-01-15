#include "stdafx.h"
#include "t_graphics_rendermanager.h"
#include <MapHelper.hpp>

constexpr mu_uint32 CommandListsIncr = 4096;

NRenderManager::NRenderManager()
{
	CommandLists.resize(CommandListsIncr);
	DraftCommandList = &CommandLists[0];
}

void NRenderManager::Execute(Diligent::IDeviceContext *immediateContext)
{
	if (Index > 1)
	{
		std::stable_sort(
			CommandLists.begin(),
			CommandLists.begin() + Index,
			[](const RCommandList &lhs, const RCommandList &rhs) -> bool { return lhs.Id < rhs.Id; }
		);
	}

	if (StateTransitions.empty() == false)
	{
		immediateContext->TransitionResourceStates(static_cast<mu_uint32>(StateTransitions.size()), StateTransitions.data());
	}

	for (auto &commandList : CommandLists)
	{
		std::vector<Diligent::StateTransitionDesc> stateTransitions;
		for (auto &command : commandList.Commands)
		{
			switch (command.Type)
			{
			case NRenderCommandType::UpdateBuffer:
				{
					auto &data = command.updateBuffer;
					immediateContext->UpdateBuffer(data.Buffer, data.Offset, data.Size, data.Data, data.StateTransitionMode);
					if (data.ShouldReleaseMemory)
					{
						mu_free(data.Data);
					}
					if (data.TransitionState != Diligent::RESOURCE_STATE_UNKNOWN)
					{
						stateTransitions.push_back(Diligent::StateTransitionDesc(data.Buffer, Diligent::RESOURCE_STATE_COPY_DEST, data.TransitionState, data.TransitionFlags));
					}
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
					if (data.SubresData.pData != nullptr && data.ShouldReleaseMemory)
					{
						mu_free(const_cast<void*>(data.SubresData.pData));
					}
					if (data.TransitionState != Diligent::RESOURCE_STATE_UNKNOWN)
					{
						stateTransitions.push_back(Diligent::StateTransitionDesc(data.Texture, Diligent::RESOURCE_STATE_COPY_DEST, data.TransitionState, data.TransitionFlags));
					}
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
					auto stateTransitionMode = (
						data.ShaderResourceBinding->ShouldTransition
						? Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION
						: data.StateTransitionMode != Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE
						? data.StateTransitionMode
						: Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY
					);
					immediateContext->CommitShaderResources(data.ShaderResourceBinding->Binding, stateTransitionMode);
					data.ShaderResourceBinding->ShouldTransition = false;
				}
				break;

			case NRenderCommandType::Draw:
				{
					if (stateTransitions.empty() == false)
					{
						immediateContext->TransitionResourceStates(static_cast<mu_uint32>(stateTransitions.size()), stateTransitions.data());
					}

					auto &data = command.draw;
					immediateContext->Draw(data.Attribs);
				}
				break;

			case NRenderCommandType::DrawIndexed:
				{
					if (stateTransitions.empty() == false)
					{
						immediateContext->TransitionResourceStates(static_cast<mu_uint32>(stateTransitions.size()), stateTransitions.data());
					}

					auto &data = command.drawIndexed;
					immediateContext->DrawIndexed(data.Attribs);
				}
				break;
			}
		}

		commandList.Commands.clear();
	}

	Index = 0;
	DraftCommandList = &CommandLists[Index];
	StateTransitions.clear();
}

void NRenderManager::TransitionResourceState(const Diligent::StateTransitionDesc &transition)
{
	StateTransitions.push_back(transition);
}

void NRenderManager::UpdateBuffer(const RUpdateBuffer &data)
{
	auto commandList = DraftCommandList;
	commandList->Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::UpdateBuffer,
			.updateBuffer = data,
		}
	);
}

void NRenderManager::UpdateBufferWithMap(const RUpdateBufferWithMap &data)
{
	auto commandList = DraftCommandList;
	commandList->Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::UpdateBufferWithMap,
			.updateBufferWithMap = data,
		}
	);
}

void NRenderManager::UpdateTexture(const RUpdateTexture &data)
{
	auto commandList = DraftCommandList;
	commandList->Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::UpdateTexture,
			.updateTexture = data,
		}
	);
}

void NRenderManager::SetDynamicTexture(const RSetDynamicTexture &data)
{
	auto commandList = DraftCommandList;
	commandList->Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::SetDynamicTexture,
			.setDynamicTexture = data,
		}
	);
}

void NRenderManager::SetDynamicBuffer(const RSetDynamicBuffer &data)
{
	auto commandList = DraftCommandList;
	commandList->Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::SetDynamicBuffer,
			.setDynamicBuffer = data,
		}
	);
}

void NRenderManager::SetPipelineState(NPipelineState *pipeline)
{
	auto commandList = DraftCommandList;
	commandList->Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::SetPipelineState,
			.setPipelineState = RSetPipelineState{
				.Pipeline = pipeline,
			},
		}
	);
	PipelineInfo = &pipeline->Info;
}

void NRenderManager::SetVertexBuffer(const RSetVertexBuffer &data)
{
	auto commandList = DraftCommandList;
	commandList->Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::SetVertexBuffer,
			.setVertexBuffer = data,
		}
	);
}

void NRenderManager::SetIndexBuffer(const RSetIndexBuffer &data)
{
	auto commandList = DraftCommandList;
	commandList->Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::SetIndexBuffer,
			.setIndexBuffer = data,
		}
	);
}

void NRenderManager::CommitShaderResources(const RCommitShaderResources &data)
{
	auto commandList = DraftCommandList;
	commandList->Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::CommitShaderResources,
			.commitShaderResources = data,
		}
	);
}

void NRenderManager::Draw(const RDraw &data, const RCommandListInfo &info)
{
	auto commandList = DraftCommandList;
	commandList->Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::Draw,
			.draw = data,
		}
	);
	PushCommandList(info);
}

void NRenderManager::DrawIndexed(const RDrawIndexed &data, const RCommandListInfo &info)
{
	auto commandList = DraftCommandList;
	commandList->Commands.push_back(
		NRenderCommand{
			.Type = NRenderCommandType::DrawIndexed,
			.drawIndexed = data,
		}
	);
	PushCommandList(info);
}

constexpr mu_uint64 TypeMask = 0x7; // 3 bits
const mu_uint64 GetCommandListClassifiedHash(
	const mu_uint64 view,
	const mu_uint64 classify,
	const mu_uint64 index,
	const mu_uint64 shader
)
{
	constexpr mu_uint64 Type = static_cast<mu_uint64>(NDrawOrderType::Classifier);
	constexpr mu_uint64 ViewMask = 0xFF; // 8 bits
	constexpr mu_uint64 ClassifyMask = 0x3; // 2 bits
	constexpr mu_uint64 ShaderMask = 0xFFFF; // 16 bits
	constexpr mu_uint64 IndexMask = 0xFF; // 8 bits

	return ((view & ViewMask) << 56) | ((Type & TypeMask) << 53) | ((classify & ClassifyMask) << 51) | ((index & IndexMask) << 43) | ((shader & ShaderMask) << 27);
}

const mu_uint64 GetCommandListSequentialHash(
	const mu_uint64 view,
	const mu_uint32 index
)
{
	constexpr mu_uint64 Type = static_cast<mu_uint64>(NDrawOrderType::Sequential);
	constexpr mu_uint64 ViewMask = 0xFF; // 8 bits
	constexpr mu_uint64 IndexMask = 0xFFFFFFFF; // 32 bits

	return ((view & ViewMask) << 56) | ((Type & TypeMask) << 53) | (index & IndexMask);
}

void NRenderManager::PushCommandList(const RCommandListInfo &info)
{
	DraftCommandList->Id = (
		info.Type == NDrawOrderType::Classifier
		? GetCommandListClassifiedHash(
			info.View,
			info.Classify == NRenderClassify::None
			? static_cast<mu_uint64>(GetRenderClassify(PipelineInfo->DepthWrite, PipelineInfo->BlendEnable, PipelineInfo->SrcBlend, PipelineInfo->DestBlend, PipelineInfo->BlendHash))
			: static_cast<mu_uint64>(info.Classify),
			PipelineInfo->Shader,
			info.Index
		)
		: GetCommandListSequentialHash(
			info.View,
			info.Index
		)
	);

	const auto index = ++Index;
	if (index >= static_cast<mu_uint32>(CommandLists.size()))
		CommandLists.resize(CommandLists.size() + CommandListsIncr);
	DraftCommandList = &CommandLists[index];
}