#include "stdafx.h"
#include "mu_renderstate.h"
#include "mu_environment.h"
#include "mu_camera.h"
#include "mu_graphics.h"
#include <MapHelper.hpp>

namespace MURenderState
{
	cglm::mat4 Projection, View, ViewProjection;
	NCamera *Camera = nullptr;
	NEnvironment *Environment = nullptr;
	std::array<NGraphicsTexture *, TextureAttachment::Count> Textures = { {} };

	Diligent::RefCntAutoPtr<Diligent::IBuffer> ProjUniform;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> ViewProjUniform;

	const mu_boolean Initialize()
	{
		const auto device = MUGraphics::GetDevice();

		// Projection Uniform
		{
			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
			bufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			bufferDesc.Size = sizeof(cglm::mat4);

			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, nullptr, &buffer);
			if (buffer == nullptr)
			{
				return false;
			}

			ProjUniform = buffer;
		}

		// View Projection Uniform
		{
			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
			bufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			bufferDesc.Size = sizeof(cglm::mat4);

			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, nullptr, &buffer);
			if (buffer == nullptr)
			{
				return false;
			}

			ViewProjUniform = buffer;
		}

		return true;
	}

	void Destroy()
	{
		ProjUniform.Release();
		ViewProjUniform.Release();
	}

	void Reset()
	{
		Environment = nullptr;
		for (mu_uint32 n = 0; n < TextureAttachment::Count; ++n)
		{
			Textures[n] = nullptr;
		}
	}

	Diligent::IBuffer *GetProjUniform()
	{
		return ProjUniform.RawPtr();
	}

	Diligent::IBuffer *GetViewProjUniform()
	{
		return ViewProjUniform.RawPtr();
	}

	void SetViewTransform(cglm::mat4 view, cglm::mat4 projection)
	{
		cglm::glm_mat4_copy(projection, Projection);
		cglm::glm_mat4_copy(view, View);
		cglm::glm_mat4_mul(projection, view, ViewProjection);

		const auto immediateContext = MUGraphics::GetImmediateContext();

		// Update
		{
			Diligent::MapHelper<cglm::mat4> uniform(immediateContext, ProjUniform, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			cglm::glm_mat4_copy(Projection, *uniform);
			cglm::glm_mat4_transpose(*uniform);
		}

		// Update
		{
			Diligent::MapHelper<cglm::mat4> uniform(immediateContext, ViewProjUniform, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			cglm::glm_mat4_copy(ViewProjection, *uniform);
			cglm::glm_mat4_transpose(*uniform);
		}
	}

	void GetViewProjection(cglm::mat4 dest)
	{
		cglm::glm_mat4_copy(ViewProjection, dest);
	}

	void GetProjection(cglm::mat4 dest)
	{
		cglm::glm_mat4_copy(Projection, dest);
	}

	void GetView(cglm::mat4 dest)
	{
		cglm::glm_mat4_copy(View, dest);
	}

	void AttachCamera(NCamera *camera)
	{
		Camera = camera;
	}

	void DetachCamera()
	{
		Camera = nullptr;
	}

	void AttachEnvironment(NEnvironment *environment)
	{
		Environment = environment;
	}

	void DetachEnvironment()
	{
		Environment = nullptr;
	}

	const NCamera *GetCamera()
	{
		return Camera;
	}

	const NEnvironment *GetEnvironment()
	{
		return Environment;
	}

	NTerrain *GetTerrain()
	{
		if (Environment == nullptr) return nullptr;
		return Environment->GetTerrain();
	}

	void AttachTexture(TextureAttachment::Type type, NGraphicsTexture *texture)
	{
		Textures[type] = texture;
	}

	void DetachTexture(TextureAttachment::Type type)
	{
		Textures[type] = nullptr;
	}

	NGraphicsTexture *GetTexture(TextureAttachment::Type type)
	{
		return Textures[type];
	}
}