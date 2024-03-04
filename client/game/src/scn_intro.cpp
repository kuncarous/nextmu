#include "mu_precompiled.h"
#include "scn_intro.h"
#include "scn_login.h"
#include "mu_scenemanager.h"
#include "mu_resourcesmanager.h"
#include "mu_charactersmanager.h"
#include "mu_updatemanager.h"
#include "mu_renderstate.h"
#include "mu_graphics.h"
#include "mu_state.h"
#include "mu_input.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
#include "ui_noesisgui.h"
#include "ngui_context.h"
#elif NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
#include "ui_rmlui.h"
#endif

mu_boolean NIntroScene::Load()
{
	std::unique_ptr<NResourcesManager> manager(new_nothrow NResourcesManager());
	if (manager->Load(GameDataPathUTF8 + "update_resources.json") == false)
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

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
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
#elif NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
	if (UIRmlUI::Initialize() == false)
	{
		mu_error("Failed to initialize rmlui.");
		return false;
	}

	if (UIRmlUI::CreateView("Intro/index.rml") == false)
	{
		mu_error("Failed to create rmlui view.");
		return false;
	}
#endif

	if (MUUpdateManager::Initialize() == false)
	{
		mu_error("Failed to initialize update manager.");
		return false;
	}

	/*if (MUCharactersManager::Load() == false)
	{
		mu_error("Failed to load characters.");
		return false;
	}

	if (MUAnimationsManager::Load() == false)
	{
		mu_error("Failed to load animations.");
		return false;
	}

	if (MUModelRenderer::Initialize() == false)
	{
		mu_error("Failed to initialize model renderer.");
		return false;
	}

	if (MUBBoxRenderer::Initialize() == false)
	{
		mu_error("Failed to initialize model renderer.");
		return false;
	}

	if (MURendersManager::Initialize() == false)
	{
		return false;
	}

	if (MUItemsManager::Initialize() == false)
	{
		return false;
	}*/

	return true;
}

void NIntroScene::Unload()
{
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
	UINoesis::GetContext()->SetUpdate(nullptr);
#endif

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

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
	auto *context = UINoesis::GetContext()->GetUpdate();
#endif

	if (FinishedUpdate == false)
	{
		FinishedUpdate = MUUpdateManager::Run();
		if (FinishedUpdate == true)
		{
			std::unique_ptr<NResourcesManager> manager(new_nothrow NResourcesManager());
			if (manager->Load(GameDataPathUTF8 + "resources.json") == false)
			{
				mu_error("Failed to load update resources json.");
				MUUpdateManager::WriteVersion(true);
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
				context->SetState("UnexpectedError");
#endif
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
		GameResources->Run();
		MUResourcesManager::SetResourcesManager(UpdateResources.get());

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
		context->SetProgress(static_cast<mu_float>(GameResources->GetResourcesLoaded()) / static_cast<mu_float>(GameResources->GetResourcesToLoad()));
#endif

		if (GameResources->IsResourcesQueueEmpty() == true)
		{
			CanStartGame = true;
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
			context->SetState("Finished");
#endif
		}
	}
	else if (CanStartGame == true && (MUInput::IsAnyMousePressing() || MUInput::IsAnyKeyPressing()))
	{
		MUSceneManager::SetQueueScene(std::make_unique<NLoginScene>());
	}

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
	UINoesis::Update();
#elif NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
	UIRmlUI::Update();
#endif

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
	UINoesis::RenderOffscreen();
#endif

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

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
	UINoesis::RenderOnscreen();
#elif NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
	UIRmlUI::RenderOnscreen();
#endif
}