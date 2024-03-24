#include "mu_precompiled.h"
#include "scn_login.h"
#include "mu_scenemanager.h"
#include "mu_resourcesmanager.h"
#include "mu_skeletonmanager.h"
#include "mu_charactersmanager.h"
#include "mu_sessionmanager.h"
#include "mu_browsermanager.h"
#include "mu_renderstate.h"
#include "mu_graphics.h"
#include "mu_state.h"
#include "mu_input.h"
#include "ui_noesisgui.h"
#include "ngui_context.h"

mu_boolean NLoginScene::Load()
{
	MUResourcesManager::SetResourcesManager(GameResources.get());

	if (UINoesis::CreateView("Login/MainWindow.xaml") == false)
	{
		mu_error("Failed to create noesisgui view.");
		return false;
	}

	LoginContext = Noesis::MakePtr<NGLoginContext>();
	UINoesis::GetContext()->SetLogin(LoginContext);

	NEnvironmentPtr environment(new NEnvironment());
	if (environment->Initialize() == false || environment->LoadTerrain("data/worlds/selectserver") == false)
	{
		mu_error("Failed to initialize login environment.");
		return false;
	}

	environment->Reset(true);
	auto *camera = environment->GetController()->GetCamera();
	camera->SetCanDrag(false);
	camera->SetMode(NCameraMode::DirectionalByAngle);
	camera->SetEye(glm::vec3(24475.796875f, 7581.581055f, 1834.539917f));
	camera->SetAngle(glm::vec3(glm::radians(275.5f), glm::radians(315.0f), glm::radians(0.0f)));
	camera->SetMinDistance(10.0f);
	camera->SetMaxDistance(30000.0f);

	Environment = std::move(environment);

	if (MURenderState::Initialize() == false)
	{
		mu_error("Failed to initialize render state.");
		return false;
	}

	if (MUSessionManager::Initialize(NEXTMU_AUTH_REALM_URL) == false)
	{
		mu_error("Failed to initialize session manager.");
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

void NLoginScene::Unload()
{
	Environment->Destroy();
	UINoesis::GetContext()->SetLogin(nullptr);
}
    
void NLoginScene::Run()
{
	const auto device = MUGraphics::GetDevice();
	const auto swapchain = MUGraphics::GetSwapChain();
	const auto &swapchainDesc = swapchain->GetDesc();
	const auto immediateContext = MUGraphics::GetImmediateContext();

	auto *pRTV = swapchain->GetCurrentBackBufferRTV();
	auto *pDSV = swapchain->GetDepthBufferDSV();

	static glm::vec3 cameraAngle(-84.5f, -45.0f, 0.0f);

	NEnvironment *environment = Environment.get();
	auto *camera = environment->GetController()->GetCamera();
	environment->Reset();
	MURenderState::AttachEnvironment(environment);
	environment->Update();

	MUSkeletonManager::Update();

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
	environment->Render();

	ShaderResourcesBindingManager.MergeTemporaryShaderBindings();

	UINoesis::RenderOnscreen();
}