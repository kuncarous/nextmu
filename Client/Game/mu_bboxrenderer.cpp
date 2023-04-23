#include "stdafx.h"
#include "mu_bboxrenderer.h"
#include "mu_resourcesmanager.h"
#include "mu_graphics.h"
#include "mu_renderstate.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <MapHelper.hpp>

#pragma pack(4)
struct NBBoxDimensions
{
	glm::vec4 Min;
	glm::vec4 Max;
};
#pragma pack()

namespace MUBBoxRenderer
{
	mu_shader BoundingBoxProgram = NInvalidShader;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> BBoxDimensionsUniform;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> VertexBuffer;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> IndexBuffer;
	NPipelineState *PipelineState = nullptr;
	Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> ShaderBinding;

	constexpr mu_uint32 NumIndexes = 6 * 6;
	const mu_boolean Initialize()
	{
		const auto device = MUGraphics::GetDevice();

		BoundingBoxProgram = MUResourcesManager::GetProgram("boundingbox");

		// BBox Dimensions
		{
			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
			bufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			bufferDesc.Size = sizeof(NBBoxDimensions);

			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, nullptr, &buffer);
			if (buffer == nullptr)
			{
				return false;
			}

			BBoxDimensionsUniform = buffer;
		}

		// Vertex Buffer
		{
			std::unique_ptr<NBBoxVertex[]> memory(new (std::nothrow) NBBoxVertex[8]);
			NBBoxVertex *vertices = reinterpret_cast<NBBoxVertex *>(memory.get());
			vertices[0].Position = glm::vec3(0.0f, 0.0f, 0.0f);
			vertices[1].Position = glm::vec3(1.0f, 0.0f, 0.0f);
			vertices[2].Position = glm::vec3(1.0f, 1.0f, 0.0f);
			vertices[3].Position = glm::vec3(0.0f, 1.0f, 0.0f);
			vertices[4].Position = glm::vec3(0.0f, 0.0f, 1.0f);
			vertices[5].Position = glm::vec3(1.0f, 0.0f, 1.0f);
			vertices[6].Position = glm::vec3(1.0f, 1.0f, 1.0f);
			vertices[7].Position = glm::vec3(0.0f, 1.0f, 1.0f);

			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_IMMUTABLE;
			bufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
			bufferDesc.Size = sizeof(NBBoxVertex[8]);

			Diligent::BufferData bufferData;
			bufferData.pData = memory.get();
			bufferData.DataSize = sizeof(NBBoxVertex[8]);

			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, &bufferData, &buffer);
			if (buffer == nullptr)
			{
				return false;
			}

			VertexBuffer = buffer;
		}

		// Index Buffer
		{
			std::unique_ptr<mu_uint16[]> memory(new (std::nothrow) mu_uint16[6 * 6]);
			mu_uint16 *indexes = memory.get();
			indexes[0] = 0; indexes[1] = 1; indexes[2] = 2; indexes += 3;
			indexes[0] = 0; indexes[1] = 2; indexes[2] = 3; indexes += 3;

			indexes[0] = 4; indexes[1] = 5; indexes[2] = 6; indexes += 3;
			indexes[0] = 4; indexes[1] = 6; indexes[2] = 7; indexes += 3;

			indexes[0] = 0; indexes[1] = 4; indexes[2] = 7; indexes += 3;
			indexes[0] = 0; indexes[1] = 7; indexes[2] = 3; indexes += 3;

			indexes[0] = 0; indexes[1] = 4; indexes[2] = 5; indexes += 3;
			indexes[0] = 0; indexes[1] = 5; indexes[2] = 1; indexes += 3;

			indexes[0] = 7; indexes[1] = 6; indexes[2] = 2; indexes += 3;
			indexes[0] = 7; indexes[1] = 2; indexes[2] = 3; indexes += 3;

			indexes[0] = 1; indexes[1] = 5; indexes[2] = 6; indexes += 3;
			indexes[0] = 1; indexes[1] = 6; indexes[2] = 2; indexes += 3;

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
		}

		const auto swapchain = MUGraphics::GetSwapChain();
		const auto &swapchainDesc = swapchain->GetDesc();

		NFixedPipelineState fixedState;
		fixedState.CombinedShader = BoundingBoxProgram;
		fixedState.RTVFormat = swapchainDesc.ColorBufferFormat;
		fixedState.DSVFormat = swapchainDesc.DepthBufferFormat;

		NDynamicPipelineState dynamicState;
		dynamicState.CullMode = Diligent::CULL_MODE_NONE;
		dynamicState.AlphaWrite = false;
		dynamicState.DepthWrite = false;
		dynamicState.SrcBlend = Diligent::BLEND_FACTOR_ONE;
		dynamicState.DestBlend = Diligent::BLEND_FACTOR_ONE;
		dynamicState.BlendOp = Diligent::BLEND_OPERATION_ADD;

		PipelineState = GetPipelineState(fixedState, dynamicState);
		PipelineState->Pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "BBoxDimensions")->Set(BBoxDimensionsUniform);

		return true;
	}

	void Destroy()
	{
		ShaderBinding.Release();
		BBoxDimensionsUniform.Release();
		VertexBuffer.Release();
		IndexBuffer.Release();
	}

	void Render(
		NBoundingBox &bbox
	)
	{
		const auto renderManager = MUGraphics::GetRenderManager();

		renderManager->SetVertexBuffer(
			RSetVertexBuffer{
				.StartSlot = 0,
				.Buffer = VertexBuffer.RawPtr(),
				.Offset = 0,
				.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
				.Flags = Diligent::SET_VERTEX_BUFFERS_FLAG_RESET,
			}
		);
		renderManager->SetIndexBuffer(
			RSetIndexBuffer{
				.IndexBuffer = IndexBuffer,
				.ByteOffset = 0,
				.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
			}
		);

		// Update Min
		{
			auto uniform = std::make_unique<RTemporaryBuffer>(sizeof(NBBoxDimensions));
			NBBoxDimensions *dimensions = uniform->Get<NBBoxDimensions *>();
			dimensions->Min = glm::vec4(bbox.Min, 1.0f);
			dimensions->Max = glm::vec4(bbox.Max, 1.0f);
			renderManager->UpdateBufferWithMap(
				RUpdateBufferWithMap{
					.Buffer = BBoxDimensionsUniform,
					.Data = dimensions,
					.Size = sizeof(NBBoxDimensions),
					.MapType = Diligent::MAP_WRITE,
					.MapFlags = Diligent::MAP_FLAG_DISCARD,
				},
				std::move(uniform)
			);
		}

		renderManager->SetPipelineState(PipelineState);
		renderManager->CommitShaderResources(
			RCommitShaderResources{
				.ShaderResourceBinding = ShaderBinding,
				.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
			}
		);
		renderManager->DrawIndexed(
			RDrawIndexed{
				.Attribs = Diligent::DrawIndexedAttribs(NumIndexes, Diligent::VT_UINT16, Diligent::DRAW_FLAG_VERIFY_ALL)
			},
			RCommandListInfo{
				.Type = NDrawOrderType::Blend,
				.View = 10,
				.Depth = 0,
			}
		);
	}
}