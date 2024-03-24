#include "mu_precompiled.h"
#include "mu_browsermanager.h"
#include "mu_input.h"
#include "ncef_renderer.h"
#include "ncef_client.h"
#include "ncef_keycodes.h"
#include <Poco/UnicodeConverter.h>

#if NEXTMU_EMBEDDED_BROWSER == 1
#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>
#include <include/cef_life_span_handler.h>
#include <include/cef_load_handler.h>
#include <include/wrapper/cef_helpers.h>
#endif

namespace MUBrowserManager
{
#if NEXTMU_EMBEDDED_BROWSER == 1
	CefRefPtr<NBrowserRenderer> Renderer;
	CefRefPtr<NBrowserClient> Client;
	CefRefPtr<CefBrowser> Browser;
	mu_boolean MouseState[MOUSE_BUTTON_MAX] = {};
	mu_boolean KeyState[KEYBOARD_KEY_SIZE] = {};
#endif

    const mu_int32 TryInitializeAsChildren(mu_int32 argc, mu_char **argv, void *instance)
    {
#if NEXTMU_EMBEDDED_BROWSER == 1
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
        CefMainArgs args(reinterpret_cast<HINSTANCE>(instance));
#else
		CefMainArgs args(argc, argv);
#endif
        mu_int32 result = CefExecuteProcess(args, nullptr, nullptr);
        return result;
#else
        return -1;
#endif
    }

    const mu_boolean Initialize(mu_int32 argc, mu_char **argv, void *instance)
	{
#if NEXTMU_EMBEDDED_BROWSER == 1
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
		CefMainArgs args(reinterpret_cast<HINSTANCE>(instance));
#else
		CefMainArgs args(argc, argv);
#endif

		CefSettings settings;
		settings.windowless_rendering_enabled = true;
		settings.persist_session_cookies = false;
		settings.background_color = CefColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF);
		CefString(&settings.browser_subprocess_path) = ExecutablePath + "NextMUBrowser.exe";
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS || NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX
		const mu_utf8string cachePath = GamePath + "cef_cache/";
#else
		const mu_utf8string cachePath = CachePath + "cef_cache/";
#endif
		CefString(&settings.root_cache_path) = cachePath;
		CefString(&settings.resources_dir_path) = GamePath + "cef_resources/";
		CefString(&settings.locales_dir_path) = GamePath + "cef_resources/locales/";
		settings.log_severity = LOGSEVERITY_VERBOSE;
#if !defined(CEF_USE_SANDBOX)
		settings.no_sandbox = true;
#endif

		MakeAbsoluteDirectory(cachePath);

		if (!CefInitialize(args, settings, nullptr, nullptr))
		{
			return false;
		}

		if (Renderer.get() == nullptr)
		{
			Renderer = new_nothrow NBrowserRenderer();
			if (Renderer.get() == nullptr || Renderer->Initialize() == false)
			{
				return false;
			}
		}
#endif

		return true;
    }

	const mu_boolean InitializeBrowser(const mu_utf8string url)
	{
#if NEXTMU_EMBEDDED_BROWSER == 1
		if (Client.get() == nullptr)
		{
			Client = new_nothrow NBrowserClient(Renderer);
			if (Client.get() == nullptr)
			{
				return false;
			}
		}

		CefBrowserSettings browserSettings;
		browserSettings.windowless_frame_rate = 60;

		CefWindowInfo windowInfo;
		windowInfo.SetAsWindowless(kNullWindowHandle);

		CefString cefUrl;
		cefUrl.FromString(url);
		Browser = CefBrowserHost::CreateBrowserSync(windowInfo, Client, cefUrl, browserSettings, nullptr, nullptr);
		if (Browser.get() == nullptr)
		{
			return false;
		}
#endif

		return true;
	}

	const mu_boolean ReloadShaders()
	{
#if NEXTMU_EMBEDDED_BROWSER == 1
		Renderer->ReloadShaders();
#endif
		return true;
	}

    void Destroy()
	{
#if NEXTMU_EMBEDDED_BROWSER == 1
		DestroyBrowser();
		Renderer.reset();
		CefShutdown();
#endif
	}

	void DestroyBrowser()
	{
#if NEXTMU_EMBEDDED_BROWSER == 1
		if (Browser.get() == nullptr) return;
		Browser->GetHost()->CloseBrowser(true);
		while (!Client->IsCloseAllowed()) CefDoMessageLoopWork();
		Browser.reset();
		Client.reset();
#endif
	}

	void Update()
	{
#if NEXTMU_EMBEDDED_BROWSER == 1
		CefDoMessageLoopWork();
#endif
	}

	void Render()
	{
#if NEXTMU_EMBEDDED_BROWSER == 1
		if (Browser.get() == nullptr) return;
		Renderer->Render();
#endif
	}

#if NEXTMU_EMBEDDED_BROWSER == 1
	const mu_int32 DEFAULT_KEY_CODE = 0;
	const mu_int32 DEFAULT_CHAR_CODE = -1;

	struct KeyModifiers
	{
		mu_boolean shift;
		mu_boolean ctrl;
		mu_boolean alt;
		mu_boolean num_lock;
		mu_boolean caps_lock;
		mu_boolean uppercase;

		explicit KeyModifiers(const SDL_Event *event)
		{
			auto mod = event->key.keysym.mod;
			shift = (mod & KMOD_LSHIFT) || (mod & KMOD_RSHIFT);
			ctrl = (mod & KMOD_LCTRL) || (mod & KMOD_RCTRL);
			alt = (mod & KMOD_LALT) || (mod & KMOD_RALT);
			num_lock = !(mod & KMOD_NUM);
			caps_lock = static_cast<mu_boolean>(mod & KMOD_CAPS);
			uppercase = caps_lock == !shift;

			/** Set the modifiers **/
			if (event->key.state == SDL_PRESSED)
			{
				switch (event->key.keysym.sym)
				{
				case SDLK_LSHIFT:
				case SDLK_RSHIFT:
					shift = true;
					break;
				case SDLK_LCTRL:
				case SDLK_RCTRL:
					ctrl = true;
					break;
				case SDLK_LALT:
				case SDLK_RALT:
					alt = true;
					break;
				default:
					break;
				}
			}
		}

		mu_uint32 GetCode()
		{
			mu_uint32 modifiersCode = 0;
			if (shift)
			{
				modifiersCode |= EVENTFLAG_SHIFT_DOWN;
			}
			if (ctrl)
			{
				modifiersCode |= EVENTFLAG_CONTROL_DOWN;
			}
			if (alt)
			{
				modifiersCode |= EVENTFLAG_ALT_DOWN;
			}
			if (num_lock)
			{
				modifiersCode |= EVENTFLAG_NUM_LOCK_ON;
			}
			if (caps_lock)
			{
				modifiersCode |= EVENTFLAG_CAPS_LOCK_ON;
			}
			return modifiersCode;
		}
	};

	struct KeyCodes
	{
		mu_int32 windows_code;
		mu_int32 native_code;

		KeyCodes(mu_int32 windowsCode, mu_int32 nativeCode)
			: windows_code(windowsCode), native_code(nativeCode)
		{
		}
	};

	mu_int32 GetWindowsCodeFromSDL(SDL_Keycode code, KeyModifiers &modifiers)
	{
		switch (code)
		{
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			return VKEY_RETURN;
		case SDLK_ESCAPE:
			return VKEY_ESCAPE;
		case SDLK_BACKSPACE:
			return VKEY_BACK;
		case SDLK_TAB:
		case SDLK_KP_TAB:
			return VKEY_TAB;
		case SDLK_SPACE:
		case SDLK_KP_SPACE:
			return VKEY_SPACE;
		case SDLK_EXCLAIM:
			return VKEY_1;
		case SDLK_QUOTE:
		case SDLK_QUOTEDBL:
			return VKEY_OEM_7;
		case SDLK_HASH:
			return VKEY_3;
		case SDLK_PERCENT:
			return VKEY_5;
		case SDLK_DOLLAR:
			return VKEY_4;
		case SDLK_AMPERSAND:
			return VKEY_7;
		case SDLK_LEFTPAREN:
			return VKEY_9;
		case SDLK_RIGHTPAREN:
			return VKEY_0;
		case SDLK_ASTERISK:
			return VKEY_8;
		case SDLK_PLUS:
			return VKEY_OEM_PLUS;
		case SDLK_COMMA:
			return VKEY_OEM_COMMA;
		case SDLK_MINUS:
			return VKEY_OEM_MINUS;
		case SDLK_PERIOD:
			return VKEY_OEM_PERIOD;
		case SDLK_SLASH:
			return VKEY_OEM_2;
		case SDLK_0:
		case SDLK_1:
		case SDLK_2:
		case SDLK_3:
		case SDLK_4:
		case SDLK_5:
		case SDLK_6:
		case SDLK_7:
		case SDLK_8:
		case SDLK_9:
			return VKEY_0 + (code - SDLK_0);
		case SDLK_COLON:
		case SDLK_SEMICOLON:
			return VKEY_OEM_1;
		case SDLK_LESS:
			return VKEY_OEM_COMMA;
		case SDLK_EQUALS:
			return VKEY_OEM_PLUS;
		case SDLK_GREATER:
			return VKEY_OEM_PERIOD;
		case SDLK_QUESTION:
			return VKEY_OEM_2;
		case SDLK_AT:
			return VKEY_2;
		case SDLK_LEFTBRACKET:
			return VKEY_OEM_4;
		case SDLK_BACKSLASH:
			return VKEY_OEM_5;
		case SDLK_RIGHTBRACKET:
			return VKEY_OEM_6;
		case SDLK_CARET:
			return VKEY_6;
		case SDLK_UNDERSCORE:
			return VKEY_OEM_MINUS;
		case SDLK_BACKQUOTE:
			return VKEY_OEM_3;
		case SDLK_a:
		case SDLK_b:
		case SDLK_c:
		case SDLK_d:
		case SDLK_e:
		case SDLK_f:
		case SDLK_g:
		case SDLK_h:
		case SDLK_i:
		case SDLK_j:
		case SDLK_k:
		case SDLK_l:
		case SDLK_m:
		case SDLK_n:
		case SDLK_o:
		case SDLK_p:
		case SDLK_q:
		case SDLK_r:
		case SDLK_s:
		case SDLK_t:
		case SDLK_u:
		case SDLK_v:
		case SDLK_w:
		case SDLK_x:
		case SDLK_y:
		case SDLK_z:
			return VKEY_A + (code - SDLK_a);
		case SDLK_CAPSLOCK:
			return VKEY_CAPITAL;
		case SDLK_F1:
		case SDLK_F2:
		case SDLK_F3:
		case SDLK_F4:
		case SDLK_F5:
		case SDLK_F6:
		case SDLK_F7:
		case SDLK_F8:
		case SDLK_F9:
		case SDLK_F10:
		case SDLK_F11:
		case SDLK_F12:
		case SDLK_F13:
		case SDLK_F14:
		case SDLK_F15:
		case SDLK_F16:
		case SDLK_F17:
		case SDLK_F18:
		case SDLK_F19:
		case SDLK_F20:
		case SDLK_F21:
		case SDLK_F22:
		case SDLK_F23:
		case SDLK_F24:
			return VKEY_F1 + (code - SDLK_F1);
		case SDLK_PRINTSCREEN:
			return VKEY_PRINT;
		case SDLK_SCROLLLOCK:
			return VKEY_SCROLL;
		case SDLK_PAUSE:
			return VKEY_PAUSE;
		case SDLK_INSERT:
			return VKEY_INSERT;
		case SDLK_HOME:
			return VKEY_HOME;
		case SDLK_PAGEUP:
			return VKEY_PRIOR;
		case SDLK_DELETE:
		case SDLK_KP_PERIOD:
			return VKEY_DELETE;
		case SDLK_END:
			return VKEY_END;
		case SDLK_PAGEDOWN:
			return VKEY_NEXT;
		case SDLK_RIGHT:
			return VKEY_RIGHT;
		case SDLK_LEFT:
			return VKEY_LEFT;
		case SDLK_DOWN:
			return VKEY_DOWN;
		case SDLK_UP:
			return VKEY_UP;
		case SDLK_KP_DIVIDE:
			return VKEY_DIVIDE;
		case SDLK_KP_MULTIPLY:
			return VKEY_MULTIPLY;
		case SDLK_KP_MINUS:
			return VKEY_OEM_MINUS;
		case SDLK_KP_PLUS:
			return VKEY_OEM_PLUS;
		case SDLK_CLEAR:
		case SDLK_KP_CLEAR:
			return VKEY_CLEAR;
		case SDLK_KP_0:
			return modifiers.num_lock ? VKEY_INSERT : VKEY_0;
		case SDLK_KP_1:
			return modifiers.num_lock ? VKEY_END : VKEY_1;
		case SDLK_KP_2:
			return modifiers.num_lock ? VKEY_DOWN : VKEY_2;
		case SDLK_KP_3:
			return modifiers.num_lock ? VKEY_NEXT : VKEY_3;
		case SDLK_KP_4:
			return modifiers.num_lock ? VKEY_LEFT : VKEY_4;
		case SDLK_KP_5:
			return VKEY_5;
		case SDLK_KP_6:
			return modifiers.num_lock ? VKEY_RIGHT : VKEY_6;
		case SDLK_KP_7:
			return modifiers.num_lock ? VKEY_HOME : VKEY_7;
		case SDLK_KP_8:
			return modifiers.num_lock ? VKEY_UP : VKEY_8;
		case SDLK_KP_9:
			return modifiers.num_lock ? VKEY_PRIOR : VKEY_9;
		case SDLK_APPLICATION:
			return VKEY_APPS;
		case SDLK_POWER:
			return VKEY_POWER;
		case SDLK_KP_EQUALS:
			return VKEY_OEM_PLUS;
		case SDLK_EXECUTE:
			return VKEY_EXECUTE;
		case SDLK_HELP:
			return VKEY_HELP;
		case SDLK_MENU:
			return VKEY_APPS;
		case SDLK_SELECT:
			return VKEY_SELECT;
		case SDLK_STOP:
			return VKEY_BROWSER_STOP;
		case SDLK_AGAIN:
			return VKEY_BROWSER_REFRESH;
		case SDLK_UNDO:
			return VKEY_Z;
		case SDLK_CUT:
			return VKEY_X;
		case SDLK_COPY:
			return VKEY_C;
		case SDLK_PASTE:
			return VKEY_V;
		case SDLK_FIND:
			return VKEY_BROWSER_SEARCH;
		case SDLK_MUTE:
			return VKEY_VOLUME_MUTE;
		case SDLK_VOLUMEUP:
			return VKEY_VOLUME_UP;
		case SDLK_VOLUMEDOWN:
			return VKEY_VOLUME_DOWN;
		case SDLK_KP_COMMA:
			return VKEY_OEM_COMMA;
		default:
			return VKEY_OEM_MINUS;
		}
	}

	void HandleKeyEvent(const SDL_Event *event)
	{
		/** KeyModifiers **/
		KeyModifiers modifiers(event);

		/** Output codes **/
		KeyCodes keyCodes(0, 0);

		if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP)
		{
			SDL_Keycode code = event->key.keysym.sym;
			keyCodes.windows_code = GetWindowsCodeFromSDL(code, modifiers);
			keyCodes.native_code = code;

			/** Still not mapped? **/
			if (keyCodes.windows_code == 0) keyCodes.windows_code = event->key.keysym.sym;
			if (keyCodes.native_code == 0) keyCodes.native_code = event->key.keysym.sym;
		}

		/** Fire key events to CEF **/
		if (event->type == SDL_TEXTINPUT)
		{
			CefKeyEvent key_event_char;
			key_event_char.type = KEYEVENT_CHAR;
			key_event_char.modifiers = modifiers.GetCode();
			Poco::UTF16String charCode;
			Poco::UnicodeConverter::toUTF16(event->text.text, charCode);
			key_event_char.windows_key_code = charCode[0];
			key_event_char.native_key_code = keyCodes.native_code;
			key_event_char.character = charCode[0];
			key_event_char.unmodified_character = charCode[0];
			Browser->GetHost()->SendKeyEvent(key_event_char);
		}
		else if (event->key.state == SDL_PRESSED)
		{
			CefKeyEvent key_event_key_down;
			key_event_key_down.type = KEYEVENT_KEYDOWN;
			key_event_key_down.modifiers = modifiers.GetCode();
			key_event_key_down.windows_key_code = keyCodes.windows_code;
			key_event_key_down.native_key_code = keyCodes.native_code;
			Browser->GetHost()->SendKeyEvent(key_event_key_down);
		}
		else if (event->key.state == SDL_RELEASED)
		{
			CefKeyEvent key_event_key_up;
			key_event_key_up.type = KEYEVENT_KEYUP;
			key_event_key_up.modifiers = modifiers.GetCode();
			key_event_key_up.windows_key_code = keyCodes.windows_code;
			key_event_key_up.native_key_code = keyCodes.native_code;
			Browser->GetHost()->SendKeyEvent(key_event_key_up);
		}
	}

	CefBrowserHost::MouseButtonType TranslateMouseButton(SDL_MouseButtonEvent const &e)
	{
		CefBrowserHost::MouseButtonType result = MBT_LEFT;
		switch (e.button)
		{
		case SDL_BUTTON_LEFT:
		case SDL_BUTTON_X1:
			break;

		case SDL_BUTTON_MIDDLE:
			result = MBT_MIDDLE;
			break;

		case SDL_BUTTON_RIGHT:
		case SDL_BUTTON_X2:
			result = MBT_RIGHT;
			break;
		}
		return result;
	}
#endif

#if NEXTMU_EMBEDDED_BROWSER == 1
	const mu_uint32 GetMouseModifiers()
	{
		mu_uint32 modifiers = 0;
		if (KeyState[SDL_SCANCODE_LSHIFT] || KeyState[SDL_SCANCODE_RSHIFT]) modifiers |= EVENTFLAG_SHIFT_DOWN;
		if (KeyState[SDL_SCANCODE_LCTRL] || KeyState[SDL_SCANCODE_RCTRL]) modifiers |= EVENTFLAG_CONTROL_DOWN;
		if (KeyState[SDL_SCANCODE_LALT] || KeyState[SDL_SCANCODE_RALT]) modifiers |= EVENTFLAG_ALT_DOWN;
		if (MouseState[MOUSE_BUTTON_LEFT]) modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
		if (MouseState[MOUSE_BUTTON_MIDDLE]) modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
		if (MouseState[MOUSE_BUTTON_RIGHT]) modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
		return modifiers;
	}
#endif

	const mu_boolean ProcessEvent(const SDL_Event *event)
	{
#if NEXTMU_EMBEDDED_BROWSER == 1
		switch (event->type)
		{
		case SDL_KEYUP:
			{
				KeyState[event->key.keysym.scancode] = false;
			}
			break;

		case SDL_MOUSEBUTTONUP:
			{
				switch (event->button.button)
				{
				case SDL_BUTTON_LEFT: MouseState[MOUSE_BUTTON_LEFT] = false; break;
				case SDL_BUTTON_MIDDLE: MouseState[MOUSE_BUTTON_MIDDLE] = false; break;
				case SDL_BUTTON_RIGHT: MouseState[MOUSE_BUTTON_RIGHT] = false; break;
				}
			}
			break;
		}

		if (Browser.get() == nullptr) return false;
		switch (event->type)
		{
		case SDL_QUIT: return false;

		case SDL_WINDOWEVENT:
			switch (event->window.event)
			{
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				Renderer->OnResize(event->window.data1, event->window.data2);
				Browser->GetHost()->WasResized();
				return false;

			case SDL_WINDOWEVENT_FOCUS_GAINED:
				Browser->GetHost()->SetFocus(true);
				return false;

			case SDL_WINDOWEVENT_FOCUS_LOST:
				mu_zeromem(MouseState, sizeof(MouseState));
				mu_zeromem(KeyState, sizeof(KeyState));
				Browser->GetHost()->SetFocus(false);
				return false;

			case SDL_WINDOWEVENT_HIDDEN:
			case SDL_WINDOWEVENT_MINIMIZED:
				Browser->GetHost()->WasHidden(true);
				return false;

			case SDL_WINDOWEVENT_SHOWN:
			case SDL_WINDOWEVENT_RESTORED:
				Browser->GetHost()->WasHidden(false);
				return false;

			case SDL_WINDOWEVENT_CLOSE: return false;
			}
			break;

		case SDL_TEXTINPUT:
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			{
				if (event->type == SDL_KEYDOWN)
				{
					KeyState[event->key.keysym.scancode] = true;
				}

				HandleKeyEvent(event);
			}
			break;

		case SDL_MOUSEMOTION:
			{
				CefMouseEvent cevent;
				cevent.x = event->motion.x;
				cevent.y = event->motion.y;
				cevent.modifiers = GetMouseModifiers();

				Browser->GetHost()->SendMouseMoveEvent(cevent, false);
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			{
				if (event->type == SDL_MOUSEBUTTONDOWN)
				{
					switch (event->button.button)
					{
					case SDL_BUTTON_LEFT: MouseState[MOUSE_BUTTON_LEFT] = true; break;
					case SDL_BUTTON_MIDDLE: MouseState[MOUSE_BUTTON_MIDDLE] = true; break;
					case SDL_BUTTON_RIGHT: MouseState[MOUSE_BUTTON_RIGHT] = true; break;
					}
				}

				/** KeyModifiers **/
				KeyModifiers modifiers(event);

				CefMouseEvent cevent;
				cevent.x = event->button.x;
				cevent.y = event->button.y;
				cevent.modifiers = GetMouseModifiers();

				Browser->GetHost()->SendMouseClickEvent(cevent, TranslateMouseButton(event->button), event->type == SDL_MOUSEBUTTONUP, event->button.clicks);
			}
			break;

		case SDL_MOUSEWHEEL:
			{
				constexpr mu_int32 ScrollMultiplier = 120;
				mu_int32 delta_mul = SDL_MOUSEWHEEL_FLIPPED == event->wheel.direction ? -1 : 1;
				mu_int32 delta_x = event->wheel.x * delta_mul * ScrollMultiplier;
				mu_int32 delta_y = event->wheel.y * delta_mul * ScrollMultiplier;

				CefMouseEvent cevent;
				cevent.x = event->wheel.mouseX;
				cevent.y = event->wheel.mouseY;
				cevent.modifiers = GetMouseModifiers();

				Browser->GetHost()->SendMouseWheelEvent(cevent, delta_x, delta_y);
			}
			break;
		}

		return true;
#else
		return false;
#endif
	}
};