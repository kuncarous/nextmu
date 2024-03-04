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

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
#include "ngui_context.h"

#include "RichText.h"
#include "LocExtension.h"

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

		NS_BEGIN_COLD_REGION
			NS_REGISTER_COMPONENT(LocExtension)
			Noesis::TypeOf<RichText>();
		NS_END_COLD_REGION

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
#endif