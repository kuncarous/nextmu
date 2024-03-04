#include "mu_precompiled.h"
#include "scn_login.h"
#include "mu_scenemanager.h"
#include "mu_resourcesmanager.h"
#include "mu_skeletonmanager.h"
#include "mu_charactersmanager.h"
#include "mu_updatemanager.h"
#include "mu_renderstate.h"
#include "mu_graphics.h"
#include "mu_state.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
#include "ui_noesisgui.h"
#include "ngui_context.h"
#elif NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
#include "ui_rmlui.h"
#endif

mu_boolean NLoginScene::Load()
{
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
	if (UINoesis::CreateView("Game/MainWindow.xaml") == false)
	{
		mu_error("Failed to create noesisgui view.");
		return false;
	}
#elif NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
	if (UIRmlUI::CreateView("Game/index.rml") == false)
	{
		mu_error("Failed to create rmlui view.");
		return false;
	}
#endif

	return true;
}

void NLoginScene::Unload()
{
    
}
    
void NLoginScene::Run()
{
	const auto device = MUGraphics::GetDevice();
	const auto swapchain = MUGraphics::GetSwapChain();
	const auto &swapchainDesc = swapchain->GetDesc();
	const auto immediateContext = MUGraphics::GetImmediateContext();

	auto *pRTV = swapchain->GetCurrentBackBufferRTV();
	auto *pDSV = swapchain->GetDepthBufferDSV();

	MUSkeletonManager::Update();

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