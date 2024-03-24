#include "mu_precompiled.h"
#include "scn_intro.h"
#include "scn_login.h"
#include "mu_scenemanager.h"
#include "mu_resourcesmanager.h"
#include "mu_charactersmanager.h"
#include "mu_animationsmanager.h"
#include "mu_browsermanager.h"
#include "mu_updatemanager.h"
#include "mu_modelrenderer.h"
#include "mu_bboxrenderer.h"
#include "res_renders.h"
#include "res_items.h"
#include "mu_renderstate.h"
#include "mu_graphics.h"
#include "mu_state.h"
#include "mu_input.h"
#include "ui_noesisgui.h"
#include "ngui_context.h"

mu_boolean NIntroScene::Load()
{
	NResourcesManagerPtr manager(new_nothrow NResourcesManager());
	if (manager->Load(GameDataPath + "update_resources.json") == false)
	{
		mu_error("Failed to load update resources json.");
		return false;
	}

	MUResourcesManager::SetResourcesManager(manager.get());

	if (manager->RunAndWait() == false)
	{
		mu_error("Failed to load update resources.");
		return false;
	}

	UpdateResources = std::move(manager);

	if (MURenderState::Initialize() == false)
	{
		mu_error("Failed to initialize render state.");
		return false;
	}

	if (UINoesis::Initialize() == false)
	{
		mu_error("Failed to initialize noesisgui.");
		return false;
	}

	if (UINoesis::CreateView("Intro/MainWindow.xaml") == false)
	{
		mu_error("Failed to create noesisgui view.");
		return false;
	}

	UpdateContext = Noesis::MakePtr<NGUpdateContext>();
	UINoesis::GetContext()->SetUpdate(UpdateContext);
	UINoesis::GetContext()->SetPopup(Noesis::MakePtr<NGPopupContext>());

	if (MUUpdateManager::Initialize() == false)
	{
		mu_error("Failed to initialize update manager.");
		return false;
	}

#if NEXTMU_EMBEDDED_BROWSER == 1
	if (MUBrowserManager::ReloadShaders() == false)
	{
		mu_error("Failed to reload embedded browser shaders.");
		return false;
	}
#endif

	return true;
}

void NIntroScene::Unload()
{
#if NEXTMU_EMBEDDED_BROWSER == 1
	//MUBrowserManager::DestroyBrowser();
#endif

	UINoesis::DeleteView();
	UINoesis::GetContext()->SetUpdate(nullptr);
	UINoesis::ResetDeviceShaders();

    MUUpdateManager::Destroy();
	if (UpdateResources != nullptr)
	{
		UpdateResources->Destroy();
		UpdateResources = nullptr;
	}
}
    
void NIntroScene::Run()
{
	const auto device = MUGraphics::GetDevice();
	const auto swapchain = MUGraphics::GetSwapChain();
	const auto &swapchainDesc = swapchain->GetDesc();
	const auto immediateContext = MUGraphics::GetImmediateContext();

	auto *pRTV = swapchain->GetCurrentBackBufferRTV();
	auto *pDSV = swapchain->GetDepthBufferDSV();

	auto *context = UINoesis::GetContext()->GetUpdate();

	if (FinishedUpdate == false)
	{
		FinishedUpdate = MUUpdateManager::Run();
		if (FinishedUpdate == true)
		{
			NResourcesManagerPtr manager(new_nothrow NResourcesManager());
			if (manager->Load(GameDataPath + "resources.json") == false)
			{
				mu_error("Failed to load update resources json.");
				MUUpdateManager::WriteVersion(true);
				context->SetState("UnexpectedError");
			}
			else
			{
				GameResources = std::move(manager);
			}
		}
	}
	else if (GameResources != nullptr && GameResources->IsResourcesQueueEmpty() == false)
	{
		MUResourcesManager::SetResourcesManager(GameResources.get());
		GameResources->RunByTime(33); // 33ms
		MUResourcesManager::SetResourcesManager(UpdateResources.get());

		context->SetProgress(static_cast<mu_float>(GameResources->GetResourcesLoaded()) / static_cast<mu_float>(GameResources->GetResourcesToLoad()));

		if (GameResources->IsResourcesQueueEmpty() == true)
		{
			mu_boolean failed = false;

			if (MUCharactersManager::Load() == false)
			{
				mu_error("Failed to load characters.");
				failed = true;
			}
			else if (MUAnimationsManager::Load() == false)
			{
				mu_error("Failed to load animations.");
				failed = true;
			}
			else if (MUModelRenderer::Initialize() == false)
			{
				mu_error("Failed to initialize model renderer.");
				failed = true;
			}
			else if (MUBBoxRenderer::Initialize() == false)
			{
				mu_error("Failed to initialize model renderer.");
				failed = true;
			}
			else if (MURendersManager::Initialize() == false)
			{
				failed = true;
			}
			else if (MUItemsManager::Initialize() == false)
			{
				failed = true;
			}

			if (failed == false)
			{
				CanStartGame = true;
				context->SetState("Finished");
			}
			else
			{
				context->SetState("UnexpectedError");
			}
		}
	}
	else if (CanStartGame == true && (MUInput::IsAnyMousePressing() || MUInput::IsAnyKeyPressing()))
	{
		MUSceneManager::SetQueueScene(std::make_unique<NLoginScene>(std::move(GameResources)));
	}

	UINoesis::Update();
	UINoesis::RenderOffscreen();

	MURenderState::SetRenderMode(NRenderMode::Normal);
	MUGraphics::SetRenderTargetDesc(
		NRenderTargetDesc{
			.ColorFormat = swapchainDesc.ColorBufferFormat,
			.DepthStencilFormat = swapchainDesc.DepthBufferFormat,
		}
	);
	immediateContext->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	mu_float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	immediateContext->ClearRenderTarget(pRTV, clearColor, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
	immediateContext->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG | Diligent::CLEAR_STENCIL_FLAG, 1.0f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	ShaderResourcesBindingManager.MergeTemporaryShaderBindings();

	UINoesis::RenderOnscreen();
}