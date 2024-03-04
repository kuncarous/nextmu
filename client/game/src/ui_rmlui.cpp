#include "mu_precompiled.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
#include "ui_rmlui.h"
#include "ui_rmlui_system.h"
#include "ui_rmlui_renderer.h"
#include "mu_config.h"
#include "mu_window.h"
#include "mu_state.h"
#include "mu_graphics.h"
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

namespace UIRmlUI
{
	std::unique_ptr<NSystemInterface> System;
	std::unique_ptr<NRenderInterface> Render;
	Rml::Context *Context;
	Rml::ElementDocument *Document = nullptr;

	const mu_boolean CreateSystem();
	const mu_boolean CreateRender();
	const mu_boolean LoadFonts();

	const mu_boolean Initialize()
	{
		if (CreateSystem() == false)
		{
			mu_error("failed to create noesisgui view");
			return false;
		}

		if (CreateRender() == false)
		{
			mu_error("failed to create noesisgui device");
			return false;
		}

		Rml::SetSystemInterface(System.get());
		Rml::SetRenderInterface(Render.get());

		Rml::Initialise();

		const auto swapchain = MUGraphics::GetSwapChain();
		auto &swapchainDesc = swapchain->GetDesc();
		Context = Rml::CreateContext("main", Rml::Vector2i(swapchainDesc.Width, swapchainDesc.Height));
		if (!Context)
		{
			return false;
		}

#ifndef NDEBUG
		Rml::Debugger::Initialise(Context);
#endif

		if (!LoadFonts())
		{
			return false;
		}

		return true;
	}

	const mu_boolean CreateView(const mu_utf8string filename)
	{
		if (Document != nullptr)
		{
			Document->Close();
			Document = nullptr;
		}

		Document = Context->LoadDocument(SupportPathUTF8 + "resources/" + filename);
		if (!Document)
		{
			return false;
		}

		Document->Show();

		return true;
	}

	void Destroy()
	{
		Rml::Shutdown();
		System.reset();
		Render.reset();
		Document = nullptr;
	}

	const mu_boolean CreateSystem()
	{
		System.reset(new NSystemInterface());
		if (!System)
		{
			return false;
		}

		return true;
	}

	const mu_boolean CreateRender()
	{
		Render.reset(new NRenderInterface());
		if (!Render)
		{
			return false;
		}

		if (Render->Initialize() == false)
		{
			return false;
		}

		return true;
	}

	const mu_boolean LoadFonts()
	{
		const Rml::String directory = SupportPathUTF8 + "data/fonts/";

		struct FontFace {
			const char *filename;
			bool fallback_face;
		};
		FontFace font_faces[] = {
			{"LatoLatin-Regular.ttf", false},
			{"LatoLatin-Italic.ttf", false},
			{"LatoLatin-Bold.ttf", false},
			{"LatoLatin-BoldItalic.ttf", false},
			{"NotoEmoji-Regular.ttf", true},
		};

		for (const FontFace &face : font_faces)
			Rml::LoadFontFace(directory + face.filename, face.fallback_face);

		return true;
	}

	void Update()
	{
		Context->Update();
	}

	void RenderOnscreen()
	{
		Render->BeginFrame();
		Context->Render();
		Render->EndFrame();
	}

	void ReleaseGarbage()
	{
		Render->ReleaseGarbage();
	}

	const mu_boolean OnEvent(const SDL_Event *event)
	{
		mu_boolean result = false;
		switch (event->type)
		{
		case SDL_MOUSEMOTION:
			result = Context->ProcessMouseMove(event->motion.x, event->motion.y, GetKeyModifierState());
			break;
		case SDL_MOUSEBUTTONDOWN:
			result = Context->ProcessMouseButtonDown(ConvertMouseButton(event->button.button), GetKeyModifierState());
			SDL_CaptureMouse(SDL_TRUE);
			break;
		case SDL_MOUSEBUTTONUP:
			SDL_CaptureMouse(SDL_FALSE);
			result = Context->ProcessMouseButtonUp(ConvertMouseButton(event->button.button), GetKeyModifierState());
			break;
		case SDL_MOUSEWHEEL:
			result = Context->ProcessMouseWheel(float(-event->wheel.y), GetKeyModifierState());
			break;
		case SDL_KEYDOWN:
			result = Context->ProcessKeyDown(ConvertKey(event->key.keysym.sym), GetKeyModifierState());
			if (event->key.keysym.sym == SDLK_RETURN || event->key.keysym.sym == SDLK_KP_ENTER)
				result &= Context->ProcessTextInput('\n');
			break;
		case SDL_KEYUP:
			result = Context->ProcessKeyUp(ConvertKey(event->key.keysym.sym), GetKeyModifierState());
			break;
		case SDL_TEXTINPUT:
			result = Context->ProcessTextInput(Rml::String(&event->text.text[0]));
			break;
		case SDL_WINDOWEVENT:
			{
				switch (event->window.event)
				{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					{
						Rml::Vector2i dimensions(event->window.data1, event->window.data2);
						Context->SetDimensions(dimensions);
					}
					break;
				case SDL_WINDOWEVENT_LEAVE:
					Context->ProcessMouseLeave();
					break;
				}
			}
			break;
		default:
			break;
		}

		return false;
	}
};
#endif