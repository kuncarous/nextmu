#include "stdafx.h"
#include "mu_renderstate.h"
#include "mu_environment.h"
#include "mu_camera.h"
#include "mu_graphics.h"
#include <MapHelper.hpp>

namespace MURenderState
{
	glm::mat4 FrustomProjection, Projection, View, ViewProjection;
	NCamera *Camera = nullptr;
	NEnvironment *Environment = nullptr;
	std::array<NGraphicsTexture *, TextureAttachment::Count> Textures = { {} };

	Diligent::RefCntAutoPtr<Diligent::IBuffer> CameraUniform;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> LightUniform;

	NRenderMode RenderMode = NRenderMode::Normal;

	NResourceId ShadowResourceId = NInvalidUInt32;
	NShadowMode ShadowMode = ShadowMapMode;
	Diligent::ShadowMapManager *ShadowMap = nullptr;

	const mu_boolean Initialize()
	{
		const auto device = MUGraphics::GetDevice();

		// Camera Uniform
		{
			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
			bufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			bufferDesc.Size = sizeof(Diligent::CameraAttribs);

			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, nullptr, &buffer);
			if (buffer == nullptr)
			{
				return false;
			}

			CameraUniform = buffer;
		}

		// Light Uniform
		{
			Diligent::BufferDesc bufferDesc;
			bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
			bufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			bufferDesc.Size = sizeof(Diligent::LightAttribs);

			Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
			device->CreateBuffer(bufferDesc, nullptr, &buffer);
			if (buffer == nullptr)
			{
				return false;
			}

			LightUniform = buffer;
		}

		return true;
	}

	void Destroy()
	{
		CameraUniform.Release();
		LightUniform.Release();
	}

	void Reset()
	{
		Environment = nullptr;
		for (mu_uint32 n = 0; n < TextureAttachment::Count; ++n)
		{
			Textures[n] = nullptr;
		}
	}

	void SetRenderMode(const NRenderMode mode)
	{
		RenderMode = mode;
	}

	const NRenderMode GetRenderMode()
	{
		return RenderMode;
	}

	void SetShadowResourceId(const NResourceId resourceId)
	{
		ShadowResourceId = resourceId;
	}

	const NResourceId GetShadowResourceId()
	{
		return ShadowResourceId;
	}

	void SetShadowMode(const NShadowMode mode)
	{
		ShadowMode = mode;
	}

	const NShadowMode GetShadowMode()
	{
		return ShadowMode;
	}

	void SetShadowMap(Diligent::ShadowMapManager *shadowMap)
	{
		ShadowMap = shadowMap;
	}

	Diligent::ShadowMapManager *GetShadowMap()
	{
		return ShadowMap;
	}

	Diligent::IBuffer *GetCameraUniform()
	{
		return CameraUniform.RawPtr();
	}

	Diligent::IBuffer *GetLightUniform()
	{
		return LightUniform.RawPtr();
	}

	void SetViewTransform(glm::mat4 view, glm::mat4 projection, glm::mat4 frustumProjection)
	{
		FrustomProjection = frustumProjection;
		Projection = projection;
		View = view;
		ViewProjection = Projection * View;
	}

	void SetViewProjection(glm::mat4 viewProj)
	{
		ViewProjection = viewProj;
	}

	glm::mat4 &GetViewProjection()
	{
		return ViewProjection;
	}

	glm::mat4 &GetFrustumProjection()
	{
		return Projection;
	}

	glm::mat4 &GetProjection()
	{
		return Projection;
	}

	glm::mat4 &GetView()
	{
		return View;
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