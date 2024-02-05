#include "mu_precompiled.h"
#include "mu_environment.h"
#include "mu_state.h"
#include "mu_camera.h"
#include "mu_modelrenderer.h"
#include "mu_bboxrenderer.h"
#include "mu_renderstate.h"
#include "mu_threadsmanager.h"
#include "mu_graphics.h"
#include "mu_config.h"
#include "mu_capabilities.h"
#include "mu_input.h"
#include <algorithm>
#include <execution>
#include <MapHelper.hpp>
#include <chrono>

enum class NThreadMode {
	Single,
	Multi,
	MultiSTL,
};

constexpr NThreadMode ThreadMode = NThreadMode::Multi;

const mu_boolean NEnvironment::Initialize()
{
	if (CreateShadowMap() == false)
	{
		return false;
	}

	Controller.reset(new_nothrow NController(this));
	if (!Controller)
	{
		return false;
	}

	Objects.reset(new_nothrow NObjects(this));
	if (!Objects || Objects->Initialize() == false)
	{
		return false;
	}

	Characters.reset(new_nothrow NCharacters(this));
	if (!Characters || Characters->Initialize() == false)
	{
		return false;
	}

	Particles.reset(new_nothrow NParticles());
	if (!Particles || Particles->Initialize() == false)
	{
		return false;
	}

	Joints.reset(new_nothrow NJoints());
	if (!Joints || Joints->Initialize() == false)
	{
		return false;
	}

	return true;
}

void NEnvironment::Destroy()
{
	if (Particles)
	{
		Particles->Destroy();
		Particles.reset();
	}

	if (Joints)
	{
		Joints->Destroy();
		Joints.reset();
	}
}

const mu_boolean NEnvironment::CreateShadowMap()
{
	const auto device = MUGraphics::GetDevice();
	if (MUConfig::GetEnableShadows() == false)
	{
		return true;
	}

	ShadowResourceId = GenerateResourceId();
	ShadowMap.reset(new_nothrow Diligent::ShadowMapManager());
	if (!ShadowMap)
	{
		return false;
	}

	Diligent::SamplerDesc ComparisonSampler;
	ComparisonSampler.ComparisonFunc = Diligent::COMPARISON_FUNC_LESS;
	// Note: anisotropic filtering requires SampleGrad to fix artifacts at
	// cascade boundaries
	ComparisonSampler.MinFilter = Diligent::FILTER_TYPE_COMPARISON_LINEAR;
	ComparisonSampler.MagFilter = Diligent::FILTER_TYPE_COMPARISON_LINEAR;
	ComparisonSampler.MipFilter = Diligent::FILTER_TYPE_COMPARISON_LINEAR;
	NSampler *comparisonSampler = GetTextureSampler(ComparisonSampler);

	Diligent::SamplerDesc FilterableSampler;
	FilterableSampler.MinFilter = Diligent::FILTER_TYPE_ANISOTROPIC;
	FilterableSampler.MagFilter = Diligent::FILTER_TYPE_ANISOTROPIC;
	FilterableSampler.MipFilter = Diligent::FILTER_TYPE_ANISOTROPIC;
	FilterableSampler.MaxAnisotropy = LightAttribs.ShadowAttribs.iMaxAnisotropy;
	NSampler *filterableSampler = GetTextureSampler(FilterableSampler);

	Diligent::ShadowMapManager::InitInfo initInfo;
	initInfo.Format = ShadowMapDepthFormat;
	initInfo.Resolution = MUConfig::GetShadowResolution();
	initInfo.NumCascades = MUConfig::GetShadowCascadesCount();
	initInfo.ShadowMode = static_cast<mu_int32>(MUConfig::GetShadowMode());
	initInfo.pComparisonSampler = comparisonSampler->Sampler;
	initInfo.pFilterableShadowMapSampler = filterableSampler->Sampler;
	initInfo.Is32BitFilterableFmt = ShadowMapDepthFormat == Diligent::TEX_FORMAT_D32_FLOAT;
	ShadowMap->Initialize(device, nullptr, initInfo);

	ShadowFrustums.resize(initInfo.NumCascades);
	RenderSettings.ShadowFrustumsNum = initInfo.NumCascades;
	RenderSettings.ShadowFrustums = ShadowFrustums.data();

	return true;
}

void NEnvironment::Reset(const mu_boolean forceReset)
{
	const auto updateCount = MUState::GetUpdateCount();

	if (forceReset || updateCount > 0)
	{
		Terrain->Reset();
	}
}

void NEnvironment::Update()
{
	const auto updateCount = MUState::GetUpdateCount();

	Characters->Update();
	Objects->Update();
	Controller->Update();

	RenderSettings.Frustum = MURenderState::GetCamera()->GetFrustum();

	if (ShadowMap != nullptr)
	{
		LightAttribs.f4Direction = LightDirection;

		Diligent::float4 f4ExtraterrestrialSunColor = Diligent::float4(1.0f, 1.0f, 1.0f, 1.0f);
		LightAttribs.f4Intensity = f4ExtraterrestrialSunColor; // *m_fScatteringScale;
		LightAttribs.f4AmbientLight = Diligent::float4(0.2f, 0.2f, 0.2f, ShadowMapMinimumValue);

		const auto cascadesCount = MUConfig::GetShadowCascadesCount();
		const auto resolution = MUConfig::GetShadowResolution();

		LightAttribs.ShadowAttribs.iNumCascades = cascadesCount;
		if (resolution >= 2048)
			LightAttribs.ShadowAttribs.fFixedDepthBias = 0.0025f;
		else if (resolution >= 1024)
			LightAttribs.ShadowAttribs.fFixedDepthBias = 0.0050f;
		else
			LightAttribs.ShadowAttribs.fFixedDepthBias = 0.0075f;

		Diligent::float4x4 cameraView = Float4x4FromGLM(MURenderState::GetView());
		Diligent::float4x4 cameraProj = Float4x4FromGLM(MURenderState::GetShadowProjection());

		Diligent::ShadowMapManager::DistributeCascadeInfo DistrInfo;
		DistrInfo.UseRightHandedLightViewTransform = MUConfig::GetShadowRightHanded();
		DistrInfo.pCameraView = &cameraView;
		//DistrInfo.pCameraWorld = &cameraWorld;
		DistrInfo.pCameraProj = &cameraProj;
		Diligent::float3 lightDirection(LightAttribs.f4Direction.x, LightAttribs.f4Direction.y, LightAttribs.f4Direction.z);
		DistrInfo.pLightDir = &lightDirection;
		DistrInfo.fPartitioningFactor = MUConfig::GetShadowPartitioning();
		DistrInfo.AdjustCascadeRange =
			[this](int iCascade, float &MinZ, float &MaxZ) {
			if (iCascade < 0)
			{
				// Snap camera z range to the exponential scale
				const float pw = 1.1f;
				MinZ = std::pow(pw, std::floor(std::log(std::max(MinZ, 1.f)) / std::log(pw)));
				MinZ = std::max(MinZ, 10.f);
				MaxZ = std::pow(pw, std::ceil(std::log(std::max(MaxZ, 1.f)) / std::log(pw)));
			}
		};
		ShadowMap->DistributeCascades(DistrInfo, LightAttribs.ShadowAttribs);

		const auto deviceType = MUGraphics::GetDeviceType();
		const mu_boolean isGL = (
			deviceType == Diligent::RENDER_DEVICE_TYPE_GL ||
			deviceType == Diligent::RENDER_DEVICE_TYPE_GLES
			);
		for (mu_uint32 n = 0; n < cascadesCount; ++n)
		{
			const auto &cascadeProj = ShadowMap->GetCascadeTranform(n).Proj;

			auto worldToLightViewSpaceMatr = LightAttribs.ShadowAttribs.mWorldToLightViewT.Transpose();
			auto worldToLightProjSpaceMatr = worldToLightViewSpaceMatr * cascadeProj;

			Diligent::ExtractViewFrustumPlanesFromMatrix(worldToLightProjSpaceMatr, ShadowFrustums[n], isGL);
		}

		// Update Light Attribs
		{
			const auto immediateContext = MUGraphics::GetImmediateContext();
			Diligent::MapHelper<Diligent::LightAttribs> uniform(immediateContext, MURenderState::GetLightUniform(), Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			*uniform = LightAttribs;
		}
	}

	if (updateCount > 0)
	{
		Terrain->Update();
	}

	Terrain->GenerateTerrain();

	Characters->PreRender(RenderSettings);
	Controller->PreRender();
	Objects->PreRender(RenderSettings);

	Particles->Update();
	Particles->Propagate();

	Joints->Update();
	Joints->Propagate();

	Terrain->ConfigureUniforms();
}

void NEnvironment::Render()
{
	const auto immediateContext = MUGraphics::GetImmediateContext();

	const auto renderMode = MURenderState::GetRenderMode();
	switch (renderMode)
	{
	case NRenderMode::Normal:
		{
			Diligent::CameraAttribs cameraAttribs = {};
			cameraAttribs.mViewT = Float4x4FromGLM(MURenderState::GetView()).Transpose();
			cameraAttribs.mProjT = Float4x4FromGLM(MURenderState::GetProjection()).Transpose();
			cameraAttribs.mViewProjT = Float4x4FromGLM(MURenderState::GetViewProjection()).Transpose();

			// Update Camera Attribs
			{
				Diligent::MapHelper<Diligent::CameraAttribs> uniform(immediateContext, MURenderState::GetCameraUniform(), Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				*uniform = cameraAttribs;
			}

			const auto enabledShadows = MUConfig::GetEnableShadows();
			if (enabledShadows)
			{
				MURenderState::SetShadowMode(MUConfig::GetShadowMode());
				MURenderState::SetShadowResourceId(ShadowResourceId);
				MURenderState::SetShadowMap(ShadowMap.get());
			}

			Terrain->Render(RenderSettings);
			Objects->Render(RenderSettings);
			Characters->Render(RenderSettings);
			Particles->Render();
			Joints->Render();

			MUGraphics::GetRenderManager()->Execute(immediateContext);
			MUBBoxRenderer::Reset();
			MUModelRenderer::Reset();

			MURenderState::SetShadowMap(nullptr);
		}
		break;

	case NRenderMode::ShadowMap:
		{
			glm::mat4 viewProj = MURenderState::GetViewProjection();

			MUGraphics::SetRenderTargetDesc(
				NRenderTargetDesc{
					.ColorFormat = Diligent::TEX_FORMAT_UNKNOWN,
					.DepthStencilFormat = ShadowMapDepthFormat,
				}
			);

			const auto cascadesCount = MUConfig::GetShadowCascadesCount();
			const auto resolution = MUConfig::GetShadowResolution();
			for (mu_uint32 n = 0; n < cascadesCount; ++n)
			{
				//if (ShadowFrustumVisible[n] == false) continue;
				const auto &cascadeProj = ShadowMap->GetCascadeTranform(n).Proj;

				auto worldToLightViewSpaceMatr = LightAttribs.ShadowAttribs.mWorldToLightViewT.Transpose();
				auto worldToLightProjSpaceMatr = worldToLightViewSpaceMatr * cascadeProj;

				Diligent::CameraAttribs shadowCameraAttribs = {};

				shadowCameraAttribs.mViewT = LightAttribs.ShadowAttribs.mWorldToLightViewT;
				shadowCameraAttribs.mProjT = cascadeProj.Transpose();
				shadowCameraAttribs.mViewProjT = worldToLightProjSpaceMatr.Transpose();
				MURenderState::SetViewProjection(GLMFromFloat4x4(worldToLightProjSpaceMatr));

				shadowCameraAttribs.f4ViewportSize.x = static_cast<mu_float>(resolution);
				shadowCameraAttribs.f4ViewportSize.y = static_cast<mu_float>(resolution);
				shadowCameraAttribs.f4ViewportSize.z = 1.f / shadowCameraAttribs.f4ViewportSize.x;
				shadowCameraAttribs.f4ViewportSize.w = 1.f / shadowCameraAttribs.f4ViewportSize.y;

				{
					Diligent::MapHelper<Diligent::CameraAttribs> uniform(immediateContext, MURenderState::GetCameraUniform(), Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
					*uniform = shadowCameraAttribs;
				}

				auto *cascadeDSV = ShadowMap->GetCascadeDSV(n);
				immediateContext->SetRenderTargets(0, nullptr, cascadeDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
				immediateContext->ClearDepthStencil(cascadeDSV, Diligent::CLEAR_DEPTH_FLAG, 1.0f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

				RenderSettings.CurrentShadowMap = n;

				//Terrain->Render(RenderSettings); // grass shadows are ugly due to how it works
				Objects->Render(RenderSettings);
				Characters->Render(RenderSettings);

				MUGraphics::GetRenderManager()->Execute(immediateContext);
				MUBBoxRenderer::Reset();
				MUModelRenderer::Reset();
			}
			
			const Diligent::RESOURCE_STATE destState = (
				MUGraphics::GetDeviceType() == Diligent::RENDER_DEVICE_TYPE_VULKAN
				? Diligent::RESOURCE_STATE_DEPTH_READ
				: Diligent::RESOURCE_STATE_SHADER_RESOURCE
			);
			Diligent::StateTransitionDesc barrier(ShadowMap->GetCascadeDSV(0)->GetTexture(), Diligent::RESOURCE_STATE_UNKNOWN, destState, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
			immediateContext->TransitionResourceStates(1, &barrier);

			if (MUConfig::GetShadowMode() != NShadowMode::PCF)
			{
				ShadowMap->ConvertToFilterable(immediateContext, LightAttribs.ShadowAttribs);

				Diligent::StateTransitionDesc barrier(ShadowMap->GetFilterableSRV()->GetTexture(), Diligent::RESOURCE_STATE_RENDER_TARGET, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
				immediateContext->TransitionResourceStates(1, &barrier);
			}

			MURenderState::SetViewProjection(viewProj);
		}
		break;
	}
}

void NEnvironment::CalculateLight(
	const NEntity::NPosition &position,
	const NEntity::NLight &light,
	NEntity::NRenderState &renderState
) const
{
	switch (light.Mode)
	{
	case EntityLightMode::Terrain:
		{
			const auto &terrain = light.Settings.Terrain;
			const glm::vec3 terrainLight = (
				terrain.PrimaryLight
				? Terrain->CalculatePrimaryLight(position.Position[0], position.Position[1])
				: Terrain->CalculateBackLight(position.Position[0], position.Position[1])
			);
			renderState.BodyLight = glm::vec4(terrain.Color + terrainLight, 1.0f);
		}
		break;

	case EntityLightMode::Fixed:
		{
			const auto &fixed = light.Settings.Fixed;
			renderState.BodyLight = glm::vec4(fixed.Color, 1.0f);
		}
		break;

	case EntityLightMode::SinWorldTime:
		{
			const auto &worldTime = light.Settings.WorldTime;
			mu_float luminosity = glm::sin(MUState::GetWorldTime() * worldTime.TimeMultiplier) * worldTime.Multiplier + worldTime.Add;
			renderState.BodyLight = glm::vec4(luminosity, luminosity, luminosity, 1.0f);
		}
		break;
	}
}