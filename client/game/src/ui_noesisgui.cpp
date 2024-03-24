#include "mu_precompiled.h"
#include "ui_noesisgui.h"
#include "ui_noesisgui_fontprovider.h"
#include "ui_noesisgui_xamlprovider.h"
#include "ui_noesisgui_textureprovider.h"
#include "ui_noesisgui_renderdevice.h"
#include "ui_noesisgui_sdl.h"
#include "mu_config.h"
#include "mu_window.h"
#include "mu_state.h"
#include "mu_graphics.h"
#include "ngui_context.h"

// Extensions
#include "RichText.h"
#include "LocExtension.h"

// Interactivity
#include <NsApp/Interaction.h>
#include <NsApp/StyleInteraction.h>
#include <NsApp/TriggerCollection.h>
#include <NsApp/BehaviorCollection.h>
#include <NsApp/EventTrigger.h>
#include <NsApp/PropertyChangedTrigger.h>
#include <NsApp/DataTrigger.h>
#include <NsApp/DataEventTrigger.h>
#include <NsApp/KeyTrigger.h>
#include <NsApp/GamepadTrigger.h>
#include <NsApp/StoryboardCompletedTrigger.h>
#include <NsApp/TimerTrigger.h>
#include <NsApp/LoadContentAction.h>
#include <NsApp/MediaActions.h>
#include <NsApp/MouseDragElementBehavior.h>
#include <NsApp/TranslateZoomRotateBehavior.h>
#include <NsApp/ConditionBehavior.h>
#include <NsApp/ConditionalExpression.h>
#include <NsApp/ComparisonCondition.h>
#include <NsApp/GoToStateAction.h>
#include <NsApp/InvokeCommandAction.h>
#include <NsApp/ChangePropertyAction.h>
#include <NsApp/ControlStoryboardAction.h>
#include <NsApp/RemoveElementAction.h>
#include <NsApp/LaunchUriOrFileAction.h>
#include <NsApp/PlaySoundAction.h>
#include <NsApp/SetFocusAction.h>
#include <NsApp/MoveFocusAction.h>
#include <NsApp/SelectAction.h>
#include <NsApp/SelectAllAction.h>
#include <NsApp/CollectionFilterBehavior.h>
#include <NsApp/CollectionSortBehavior.h>
#include <NsApp/BackgroundEffectBehavior.h>
#include <NsApp/LineDecorationBehavior.h>

// MediaElement
#include <NsApp/MediaElement.h>

// Converters
#include "ngui_converter_viewportunit.h"

// Controls
#include "fixedwindow/fixedwindow.xaml.h"
#include "popups/popups.xaml.h"
#include "popup/popup.xaml.h"

using namespace NoesisApp;

namespace UINoesis
{
	Noesis::Ptr<Noesis::IView> View;
	Noesis::Ptr<Noesis::RenderDevice> Device;
	Noesis::Ptr<NGApplicationContext> Context;
	mu_double UpdateTime = 0.0;

	const mu_boolean CreateDevice();

	const mu_boolean Initialize()
	{
		Noesis::SetLogHandler(
			[](const char *, uint32_t, uint32_t level, const char *, const char *msg)
			{
				switch (level)
				{
				case NS_LOG_LEVEL_TRACE: mu_info("[Trace][NoesisGUI] {}", msg); break;
				case NS_LOG_LEVEL_DEBUG: mu_info("[Debug][NoesisGUI] {}", msg); break;
				case NS_LOG_LEVEL_INFO: mu_info("[Info][NoesisGUI] {}", msg); break;
				case NS_LOG_LEVEL_WARNING: mu_info("[Warning][NoesisGUI] {}", msg); break;
				case NS_LOG_LEVEL_ERROR: mu_error("[NoesisGUI] {}", msg); break;
				}
			}
		);

		Noesis::GUI::SetLicense(NS_LICENSE_NAME, NS_LICENSE_KEY);
		Noesis::GUI::Init();

		// Extensions
		Noesis::TypeOf<RichText>();
		NS_REGISTER_COMPONENT(LocExtension);

		// Interactivity
		Noesis::TypeOf<NoesisApp::Interaction>();
		Noesis::TypeOf<NoesisApp::StyleInteraction>();

		NS_REGISTER_COMPONENT(NoesisApp::BehaviorCollection);
		NS_REGISTER_COMPONENT(NoesisApp::TriggerCollection);
		NS_REGISTER_COMPONENT(NoesisApp::StyleBehaviorCollection);
		NS_REGISTER_COMPONENT(NoesisApp::StyleTriggerCollection);
		NS_REGISTER_COMPONENT(NoesisApp::EventTrigger);
		NS_REGISTER_COMPONENT(NoesisApp::PropertyChangedTrigger);
		NS_REGISTER_COMPONENT(NoesisApp::DataTrigger);
		NS_REGISTER_COMPONENT(NoesisApp::DataEventTrigger);
		NS_REGISTER_COMPONENT(NoesisApp::KeyTrigger);
		NS_REGISTER_COMPONENT(NoesisApp::GamepadTrigger);
		NS_REGISTER_COMPONENT(NoesisApp::StoryboardCompletedTrigger);
		NS_REGISTER_COMPONENT(NoesisApp::TimerTrigger);
		NS_REGISTER_COMPONENT(NoesisApp::LoadContentAction);
		NS_REGISTER_COMPONENT(NoesisApp::MouseDragElementBehavior);
		NS_REGISTER_COMPONENT(NoesisApp::TranslateZoomRotateBehavior);
		NS_REGISTER_COMPONENT(NoesisApp::ConditionBehavior);
		NS_REGISTER_COMPONENT(NoesisApp::ConditionalExpression);
		NS_REGISTER_COMPONENT(NoesisApp::ComparisonCondition);
		NS_REGISTER_COMPONENT(NoesisApp::GoToStateAction);
		NS_REGISTER_COMPONENT(NoesisApp::InvokeCommandAction);
		NS_REGISTER_COMPONENT(NoesisApp::ChangePropertyAction);
		NS_REGISTER_COMPONENT(NoesisApp::ControlStoryboardAction);
		NS_REGISTER_COMPONENT(NoesisApp::RemoveElementAction);
		NS_REGISTER_COMPONENT(NoesisApp::LaunchUriOrFileAction);
		NS_REGISTER_COMPONENT(NoesisApp::PlaySoundAction);
		NS_REGISTER_COMPONENT(NoesisApp::PlayMediaAction);
		NS_REGISTER_COMPONENT(NoesisApp::PauseMediaAction);
		NS_REGISTER_COMPONENT(NoesisApp::RewindMediaAction);
		NS_REGISTER_COMPONENT(NoesisApp::StopMediaAction);
		NS_REGISTER_COMPONENT(NoesisApp::SetFocusAction);
		NS_REGISTER_COMPONENT(NoesisApp::MoveFocusAction);
		NS_REGISTER_COMPONENT(NoesisApp::SelectAction);
		NS_REGISTER_COMPONENT(NoesisApp::SelectAllAction);
		NS_REGISTER_COMPONENT(NoesisApp::CollectionFilterBehavior);
		NS_REGISTER_COMPONENT(NoesisApp::CollectionSortBehavior);
		NS_REGISTER_COMPONENT(NoesisApp::BackgroundEffectBehavior);
		NS_REGISTER_COMPONENT(NoesisApp::LineDecorationBehavior);
		NS_REGISTER_COMPONENT(Noesis::EnumConverter<NoesisApp::ComparisonConditionType>);
		NS_REGISTER_COMPONENT(Noesis::EnumConverter<NoesisApp::ForwardChaining>);
		NS_REGISTER_COMPONENT(Noesis::EnumConverter<NoesisApp::KeyTriggerFiredOn>);
		NS_REGISTER_COMPONENT(Noesis::EnumConverter<NoesisApp::GamepadTriggerFiredOn>);
		NS_REGISTER_COMPONENT(Noesis::EnumConverter<NoesisApp::GamepadButton>);
		NS_REGISTER_COMPONENT(Noesis::EnumConverter<NoesisApp::ControlStoryboardOption>);
		NS_REGISTER_COMPONENT(Noesis::EnumConverter<NoesisApp::FocusDirection>);

		// MediaElement
		NS_REGISTER_COMPONENT(NoesisApp::MediaElement);
		NS_REGISTER_COMPONENT(Noesis::EnumConverter<NoesisApp::MediaState>);

		// Converters
		Noesis::RegisterComponent<ViewportWidthConverter>();
		Noesis::RegisterComponent<ViewportHeightConverter>();

		// Controls
		Noesis::RegisterComponent<NextMU::FixedWindow>();
		Noesis::RegisterComponent<NextMU::Popups>();
		Noesis::RegisterComponent<NextMU::Popup>();

		Noesis::GUI::SetXamlProvider(Noesis::MakePtr<XamlProvider>());
		Noesis::GUI::SetFontProvider(Noesis::MakePtr<FontProvider>());
		Noesis::GUI::SetTextureProvider(Noesis::MakePtr<TextureProvider>());

		const char *fonts[] = { "Roboto", "Exo2", "Arial" };

		Noesis::GUI::LoadApplicationResources("Theme/NoesisTheme.DarkBlue.xaml");
		Noesis::GUI::SetFontFallbacks(fonts, 3);
		Noesis::GUI::SetFontDefaultProperties(15.0f, Noesis::FontWeight_Normal, Noesis::FontStretch_Normal, Noesis::FontStyle_Normal);

		if (CreateDevice() == false)
		{
			mu_error("failed to create noesisgui device");
			return false;
		}

		Context = Noesis::MakePtr<NGApplicationContext>();

		return true;
	}

	void Destroy()
	{
		if (View && View->GetRenderer() != nullptr) View->GetRenderer()->Shutdown();
		Device.Reset();
		View.Reset();
		Context.Reset();
		Noesis::GUI::Shutdown();
	}

	NGApplicationContext *GetContext()
	{
		return Context.GetPtr();
	}

	const mu_boolean CreateView(const mu_utf8string filename)
	{
		Noesis::Ptr<Noesis::FrameworkElement> xaml = Noesis::GUI::LoadXaml<Noesis::FrameworkElement>(filename.c_str());
		if (!xaml)
		{
			mu_error("failed to load xaml");
			return false;
		}

		View = Noesis::GUI::CreateView(xaml);
		if (!View)
		{
			mu_error("failed to create view");
			return false;
		}

		const auto swapchain = MUGraphics::GetSwapChain();
		const auto &swapchainDesc = swapchain->GetDesc();

		View->SetFlags(Noesis::RenderFlags_PPAA | Noesis::RenderFlags_LCD);
		View->SetSize(swapchainDesc.Width, swapchainDesc.Height);
		View->GetRenderer()->Init(Device);
		View->GetContent()->SetDataContext(Context.GetPtr());
		UpdateTime = 0.0;
		View->Update(UpdateTime);

		return true;
	}

	void DeleteView()
	{
		if (View == nullptr) return;
		if (View->GetRenderer() != nullptr) View->GetRenderer()->Shutdown();
		View.Reset();
	}

	Noesis::IView *GetView()
	{
		return View.GetPtr();
	}

	const mu_boolean CreateDevice()
	{
		Device = Noesis::MakePtr<DERenderDevice>(MUGraphics::IssRGB());
		if (!Device)
		{
			mu_error("failed to create device");
			return false;
		}
		
		return true;
	}

	void ResetDeviceShaders()
	{
		if (!Device) return;
		auto *device = static_cast<DERenderDevice*>(Device.GetPtr());
		device->ResetShaders();
	}

	void Update()
	{
		UpdateTime += static_cast<mu_double>(MUState::GetElapsedTime());
		View->Update(UpdateTime / 1000.0);
	}

	void RenderOffscreen()
	{
		auto *renderer = View->GetRenderer();
		mu_boolean updated = renderer->UpdateRenderTree();
		renderer->RenderOffscreen();
	}

	void RenderOnscreen()
	{
		auto *renderer = View->GetRenderer();
		renderer->Render();
	}

	const mu_boolean OnEvent(const SDL_Event *event)
	{
		switch (event->type)
		{
		case SDL_MOUSEMOTION:
			{
				View->MouseMove(event->motion.x, event->motion.y);
			}
			return true;

		case SDL_MOUSEWHEEL:
			{
				if (event->wheel.x != 0)
					View->MouseHWheel(event->wheel.mouseX, event->wheel.mouseY, event->wheel.x);
				if (event->wheel.y != 0)
					View->MouseWheel(event->wheel.mouseX, event->wheel.mouseY, event->wheel.y);
			}
			return true;

		case SDL_MOUSEBUTTONDOWN:
			{
				Noesis::MouseButton button = (
					event->button.button == SDL_BUTTON_LEFT
					? Noesis::MouseButton_Left
					: event->button.button == SDL_BUTTON_RIGHT
					? Noesis::MouseButton_Right
					: event->button.button == SDL_BUTTON_MIDDLE
					? Noesis::MouseButton_Middle
					: event->button.button == SDL_BUTTON_X1
					? Noesis::MouseButton_XButton1
					: event->button.button == SDL_BUTTON_X2
					? Noesis::MouseButton_XButton2
					: Noesis::MouseButton_Count
				);
				if (button != Noesis::MouseButton_Count)
				{
					View->MouseButtonDown(
						event->button.x,
						event->button.y,
						button
					);
				}
			}
			break;

		case SDL_MOUSEBUTTONUP:
			{
				Noesis::MouseButton button = (
					event->button.button == SDL_BUTTON_LEFT
					? Noesis::MouseButton_Left
					: event->button.button == SDL_BUTTON_RIGHT
					? Noesis::MouseButton_Right
					: event->button.button == SDL_BUTTON_MIDDLE
					? Noesis::MouseButton_Middle
					: event->button.button == SDL_BUTTON_X1
					? Noesis::MouseButton_XButton1
					: event->button.button == SDL_BUTTON_X2
					? Noesis::MouseButton_XButton2
					: Noesis::MouseButton_Count
				);
				if (button != Noesis::MouseButton_Count)
				{
					View->MouseButtonUp(
						event->button.x,
						event->button.y,
						button
					);
				}
			}
			break;

		case SDL_TEXTINPUT:
			{
				View->Char(*SDL_iconv_utf8_ucs4(event->text.text));
			}
			return true;

		case SDL_KEYDOWN:
			{
				Noesis::Key key = SDLKeyCodeToNoesisKeyCode(event->key.keysym.sym);
				if (key != Noesis::Key_Count)
				{
					View->KeyDown(key);
				}
			}
			break;

		case SDL_KEYUP:
			{
				Noesis::Key key = SDLKeyCodeToNoesisKeyCode(event->key.keysym.sym);
				if (key != Noesis::Key_Count)
				{
					View->KeyUp(key);
				}
			}
			break;

		case SDL_FINGERMOTION:
			{
				mu_int32 w, h;
				SDL_GetWindowSize(MUWindow::GetWindow(), &w, &h);
				mu_int32 x = static_cast<mu_int32>(w * event->tfinger.x);
				mu_int32 y = static_cast<mu_int32>(h * event->tfinger.y);
				View->TouchMove(x, y, event->tfinger.fingerId);
			}
			break;

		case SDL_FINGERDOWN:
			{
				mu_int32 w, h;
				SDL_GetWindowSize(MUWindow::GetWindow(), &w, &h);
				mu_int32 x = static_cast<mu_int32>(w * event->tfinger.x);
				mu_int32 y = static_cast<mu_int32>(h * event->tfinger.y);
				View->TouchDown(x, y, event->tfinger.fingerId);
			}
			break;

		case SDL_FINGERUP:
			{
				mu_int32 w, h;
				SDL_GetWindowSize(MUWindow::GetWindow(), &w, &h);
				mu_int32 x = static_cast<mu_int32>(w * event->tfinger.x);
				mu_int32 y = static_cast<mu_int32>(h * event->tfinger.y);
				View->TouchUp(x, y, event->tfinger.fingerId);
			}
			break;
		}

		return false;
	}
};