#include "mu_precompiled.h"
#include "mu_bboxrenderer.h"
#include "mu_resourcesmanager.h"
#include "mu_graphics.h"
#include "mu_renderstate.h"
#include "mu_resizablequeue.h"
#include <glm/gtc/type_ptr.hpp>
#include <MapHelper.hpp>

#pragma pack(4)
typedef glm::vec3 RenderVerticesType[NOrientedBoundingBox::VerticesCount];
#pragma pack()

namespace MUBBoxRenderer
{
	mu_shader BoundingBoxProgram = NInvalidShader;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> VertexBuffer;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> IndexBuffer;
	NResizableQueue<RenderVerticesType> VerticesBuffer;

	NDynamicPipelineState DynamicState = {
		.CullMode = Diligent::CULL_MODE_NONE,
		.AlphaWrite = false,
		.DepthWrite = false,
		.SrcBlend = Diligent::BLEND_FACTOR_ONE,
		.DestBlend = Diligent::BLEND_FACTOR_ONE,
		.SrcBlendAlpha = Diligent::BLEND_FACTOR_SRC_ALPHA,
		.DestBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA,
	};

	constexpr mu_uint32 NumIndexes = 6 * 6;
	mu_boolean Initialize()
	{
		const auto device = MUGraphics::GetDevice();

		BoundingBoxProgram = MUResourcesManager::GetProgram("boundingbox");

		std::vector<Diligent::StateTransitionDesc> barriers;

		// Vertex Buffer
		{
			/*std::unique_ptr<NBBoxVertex[]> memory(new_nothrow NBBoxVertex[8]);
			NBBoxVertex *vertices = reinterpret_cast<NBBoxVertex *>(memory.get());
			vertices[0].Position = glm::vec3(0.0f, 0.0f, 0.0f);
			vertices[1].Position = glm::vec3(1.0f, 0.0f, 0.0f);
			vertices[2].Position = glm::vec3(1.0f, 1.0f, 0.0f);
			vertices[3].Position = glm::vec3(0.0f, 1.0f, 0.0f);
			vertices[4].Position = glm::vec3(0.0f, 0.0f, 1.0f);
			vertices[5].Position = glm::vec3(1.0f, 0.0f, 1.0f);
			vertices[6].Position = glm::vec3(1.0f, 1.0f, 1.0f);
			vertices[7].Position = glm::vec3(0.0f, 1.0f, 1.0f);*/

			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
			bufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			bufferDesc.Size = sizeof(RenderVerticesType);
			
			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, nullptr, &buffer);
			if (buffer == nullptr)
			{
				return false;
			}

			VertexBuffer = buffer;
			//barriers.push_back(Diligent::StateTransitionDesc(buffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE));
		}

		// Index Buffer
		{
			const std::unique_ptr<mu_uint16[]> memory(new_nothrow mu_uint16[6 * 6]);
			mu_uint16 *indexes = memory.get();
			// Left Side
			indexes[0] = 0; indexes[1] = 2; indexes[2] = 3; indexes += 3;
			indexes[0] = 0; indexes[1] = 3; indexes[2] = 1; indexes += 3;

			// Right Side
			indexes[0] = 4; indexes[1] = 6; indexes[2] = 7; indexes += 3;
			indexes[0] = 4; indexes[1] = 7; indexes[2] = 5; indexes += 3;

			// Top Side
			indexes[0] = 2; indexes[1] = 3; indexes[2] = 7; indexes += 3;
			indexes[0] = 2; indexes[1] = 7; indexes[2] = 6; indexes += 3;

			// Bottom Side
			indexes[0] = 0; indexes[1] = 1; indexes[2] = 5; indexes += 3;
			indexes[0] = 0; indexes[1] = 5; indexes[2] = 4; indexes += 3;

			// Front Side
			indexes[0] = 0; indexes[1] = 2; indexes[2] = 6; indexes += 3;
			indexes[0] = 0; indexes[1] = 6; indexes[2] = 4; indexes += 3;

			// Back Side
			indexes[0] = 1; indexes[1] = 3; indexes[2] = 7; indexes += 3;
			indexes[0] = 1; indexes[1] = 7; indexes[2] = 5; indexes += 3;

			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_IMMUTABLE;
			bufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
			bufferDesc.Size = sizeof(mu_uint16) * NumIndexes;

			Diligent::BufferData bufferData;
			bufferData.pData = memory.get();
			bufferData.DataSize = sizeof(mu_uint16) * NumIndexes;

			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, &bufferData, &buffer);
			if (buffer == nullptr)
			{
				return false;
			}

			IndexBuffer = buffer;
			barriers.push_back(Diligent::StateTransitionDesc(buffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE));
		}

		const auto immediateContext = MUGraphics::GetImmediateContext();
		immediateContext->TransitionResourceStates(static_cast<mu_uint32>(barriers.size()), barriers.data());

		return true;
	}

	void Destroy()
	{
		VertexBuffer.Release();
		IndexBuffer.Release();
	}

	void Reset()
	{
		VerticesBuffer.Reset();
	}

	void Render(const NBoundingBox &aabb)
	{
		const auto renderManager = MUGraphics::GetRenderManager();
		const auto &renderTargetDesc = MUGraphics::GetRenderTargetDesc();

		NFixedPipelineState fixedState = {
			.CombinedShader = BoundingBoxProgram,
			.RTVFormat = renderTargetDesc.ColorFormat,
			.DSVFormat = renderTargetDesc.DepthStencilFormat,
		};

		auto pipelineState = GetPipelineState(fixedState, DynamicState);
		if (pipelineState->StaticInitialized == false)
		{
			pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "cbCameraAttribs")->Set(MURenderState::GetCameraUniform());
			pipelineState->StaticInitialized = true;
		}

		NResourceId resourceIds[1] = { NInvalidUInt32 };
		auto binding = ShaderResourcesBindingManager.GetShaderBinding(pipelineState->Id, pipelineState->Pipeline, mu_countof(resourceIds), resourceIds);
		binding->Initialized = true;

		renderManager->SetVertexBuffer(
			RSetVertexBuffer{
				.StartSlot = 0,
				.Buffer = VertexBuffer.RawPtr(),
				.Offset = 0,
				.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY,
				.Flags = Diligent::SET_VERTEX_BUFFERS_FLAG_NONE,
			}
		);
		renderManager->SetIndexBuffer(
			RSetIndexBuffer{
				.IndexBuffer = IndexBuffer,
				.ByteOffset = 0,
				.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY,
			}
		);

		// Update Vertices
		{
			auto &vertices = *VerticesBuffer.Allocate();

			vertices[0] = glm::vec3(aabb.Min.x, aabb.Min.y, aabb.Min.z);
			vertices[1] = glm::vec3(aabb.Min.x, aabb.Min.y, aabb.Max.z);
			vertices[2] = glm::vec3(aabb.Min.x, aabb.Max.y, aabb.Min.z);
			vertices[3] = glm::vec3(aabb.Min.x, aabb.Max.y, aabb.Max.z);
			vertices[4] = glm::vec3(aabb.Max.x, aabb.Min.y, aabb.Min.z);
			vertices[5] = glm::vec3(aabb.Max.x, aabb.Min.y, aabb.Max.z);
			vertices[6] = glm::vec3(aabb.Max.x, aabb.Max.y, aabb.Min.z);
			vertices[7] = glm::vec3(aabb.Max.x, aabb.Max.y, aabb.Max.z);

			renderManager->UpdateBufferWithMap(
				RUpdateBufferWithMap{
					.ShouldReleaseMemory = false,
					.Buffer = VertexBuffer,
					.Data = vertices,
					.Size = sizeof(RenderVerticesType),
					.MapType = Diligent::MAP_WRITE,
					.MapFlags = Diligent::MAP_FLAG_DISCARD,
				}
			);
		}

		renderManager->SetPipelineState(pipelineState);
		renderManager->CommitShaderResources(
			RCommitShaderResources{
				.ShaderResourceBinding = binding,
			}
		);
		renderManager->DrawIndexed(
			RDrawIndexed{
				.Attribs = Diligent::DrawIndexedAttribs(NumIndexes, Diligent::VT_UINT16, Diligent::DRAW_FLAG_VERIFY_ALL)
			},
			RCommandListInfo{
				.Type = NDrawOrderType::Classifier,
				.Classify = NRenderClassify::PostAlpha,
				.View = 10,
				.Index = 0,
			}
		);
	}

	void Render(const NOrientedBoundingBox &obb)
	{
		const auto renderManager = MUGraphics::GetRenderManager();
		const auto &renderTargetDesc = MUGraphics::GetRenderTargetDesc();

		NFixedPipelineState fixedState = {
			.CombinedShader = BoundingBoxProgram,
			.RTVFormat = renderTargetDesc.ColorFormat,
			.DSVFormat = renderTargetDesc.DepthStencilFormat,
		};

		auto pipelineState = GetPipelineState(fixedState, DynamicState);
		if (pipelineState->StaticInitialized == false)
		{
			pipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "cbCameraAttribs")->Set(MURenderState::GetCameraUniform());
			pipelineState->StaticInitialized = true;
		}

		NResourceId resourceIds[1] = { NInvalidUInt32 };
		auto binding = ShaderResourcesBindingManager.GetShaderBinding(pipelineState->Id, pipelineState->Pipeline, mu_countof(resourceIds), resourceIds);
		binding->Initialized = true;

		renderManager->SetVertexBuffer(
			RSetVertexBuffer{
				.StartSlot = 0,
				.Buffer = VertexBuffer.RawPtr(),
				.Offset = 0,
				.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY,
				.Flags = Diligent::SET_VERTEX_BUFFERS_FLAG_NONE,
			}
		);
		renderManager->SetIndexBuffer(
			RSetIndexBuffer{
				.IndexBuffer = IndexBuffer,
				.ByteOffset = 0,
				.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY,
			}
		);

		// Update Vertices
		{
			auto vertices = VerticesBuffer.Allocate();

			mu_memcpy(vertices, obb.Vertices, sizeof(obb.Vertices));

			renderManager->UpdateBufferWithMap(
				RUpdateBufferWithMap{
					.ShouldReleaseMemory = false,
					.Buffer = VertexBuffer,
					.Data = vertices,
					.Size = sizeof(RenderVerticesType),
					.MapType = Diligent::MAP_WRITE,
					.MapFlags = Diligent::MAP_FLAG_DISCARD,
				}
			);
		}

		renderManager->SetPipelineState(pipelineState);
		renderManager->CommitShaderResources(
			RCommitShaderResources{
				.ShaderResourceBinding = binding,
			}
		);
		renderManager->DrawIndexed(
			RDrawIndexed{
				.Attribs = Diligent::DrawIndexedAttribs(NumIndexes, Diligent::VT_UINT16, Diligent::DRAW_FLAG_VERIFY_ALL)
			},
			RCommandListInfo{
				.Type = NDrawOrderType::Classifier,
				.Classify = NRenderClassify::PostAlpha,
				.View = 10,
				.Index = 0,
			}
		);
	}
}