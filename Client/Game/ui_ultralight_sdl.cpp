#include "stdafx.h"
#include "ui_ultralight_sdl.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_ULTRALIGHT
#include <Ultralight/Ultralight.h>

mu_int32 SDLModsToUltralightMods(mu_uint16 mods)
{
	mu_int32 result = 0;
	if (mods & KMOD_ALT)
		result |= ultralight::KeyEvent::kMod_AltKey;
	if (mods & KMOD_CTRL)
		result |= ultralight::KeyEvent::kMod_CtrlKey;
	if (mods & KMOD_GUI)
		result |= ultralight::KeyEvent::kMod_MetaKey;
	if (mods & KMOD_SHIFT)
		result |= ultralight::KeyEvent::kMod_ShiftKey;
	return result;
}

mu_int32 SDLKeyCodeToUltralightKeyCode(SDL_Keycode key) 
{
	using namespace ultralight::KeyCodes;

	switch (key) {
	case SDLK_SPACE: return GK_SPACE;
	case SDLK_QUOTE: return GK_OEM_7;
	case SDLK_COMMA: return GK_OEM_COMMA;
	case SDLK_MINUS: return GK_OEM_MINUS;
	case SDLK_PERIOD: return GK_OEM_PERIOD;
	case SDLK_SLASH: return GK_OEM_2;
	case SDLK_0: return GK_0;
	case SDLK_1: return GK_1;
	case SDLK_2: return GK_2;
	case SDLK_3: return GK_3;
	case SDLK_4: return GK_4;
	case SDLK_5: return GK_5;
	case SDLK_6: return GK_6;
	case SDLK_7: return GK_7;
	case SDLK_8: return GK_8;
	case SDLK_9: return GK_9;
	case SDLK_SEMICOLON: return GK_OEM_1;
	case SDLK_EQUALS: return GK_OEM_PLUS;
	case SDLK_a: return GK_A;
	case SDLK_b: return GK_B;
	case SDLK_c: return GK_C;
	case SDLK_d: return GK_D;
	case SDLK_e: return GK_E;
	case SDLK_f: return GK_F;
	case SDLK_g: return GK_G;
	case SDLK_h: return GK_H;
	case SDLK_i: return GK_I;
	case SDLK_j: return GK_J;
	case SDLK_k: return GK_K;
	case SDLK_l: return GK_L;
	case SDLK_m: return GK_M;
	case SDLK_n: return GK_N;
	case SDLK_o: return GK_O;
	case SDLK_p: return GK_P;
	case SDLK_q: return GK_Q;
	case SDLK_r: return GK_R;
	case SDLK_s: return GK_S;
	case SDLK_t: return GK_T;
	case SDLK_u: return GK_U;
	case SDLK_v: return GK_V;
	case SDLK_w: return GK_W;
	case SDLK_x: return GK_X;
	case SDLK_y: return GK_Y;
	case SDLK_z: return GK_Z;
	case SDLK_LEFTBRACKET: return GK_OEM_4;
	case SDLK_BACKSLASH: return GK_OEM_5;
	case SDLK_RIGHTBRACKET: return GK_OEM_6;
	case SDLK_BACKQUOTE: return GK_OEM_3;
	case SDLK_ESCAPE: return GK_ESCAPE;
	case SDLK_RETURN: return GK_RETURN;
	case SDLK_TAB: return GK_TAB;
	case SDLK_BACKSPACE: return GK_BACK;
	case SDLK_INSERT: return GK_INSERT;
	case SDLK_DELETE: return GK_DELETE;
	case SDLK_RIGHT: return GK_RIGHT;
	case SDLK_LEFT: return GK_LEFT;
	case SDLK_DOWN: return GK_DOWN;
	case SDLK_UP: return GK_UP;
	case SDLK_PAGEUP: return GK_PRIOR;
	case SDLK_PAGEDOWN: return GK_NEXT;
	case SDLK_HOME: return GK_HOME;
	case SDLK_END: return GK_END;
	case SDLK_CAPSLOCK: return GK_CAPITAL;
	case SDLK_SCROLLLOCK: return GK_SCROLL;
	case SDLK_NUMLOCKCLEAR: return GK_NUMLOCK;
	case SDLK_PRINTSCREEN: return GK_SNAPSHOT;
	case SDLK_PAUSE: return GK_PAUSE;
	case SDLK_F1: return GK_F1;
	case SDLK_F2: return GK_F2;
	case SDLK_F3: return GK_F3;
	case SDLK_F4: return GK_F4;
	case SDLK_F5: return GK_F5;
	case SDLK_F6: return GK_F6;
	case SDLK_F7: return GK_F7;
	case SDLK_F8: return GK_F8;
	case SDLK_F9: return GK_F9;
	case SDLK_F10: return GK_F10;
	case SDLK_F11: return GK_F11;
	case SDLK_F12: return GK_F12;
	case SDLK_F13: return GK_F13;
	case SDLK_F14: return GK_F14;
	case SDLK_F15: return GK_F15;
	case SDLK_F16: return GK_F16;
	case SDLK_F17: return GK_F17;
	case SDLK_F18: return GK_F18;
	case SDLK_F19: return GK_F19;
	case SDLK_F20: return GK_F20;
	case SDLK_F21: return GK_F21;
	case SDLK_F22: return GK_F22;
	case SDLK_F23: return GK_F23;
	case SDLK_F24: return GK_F24;
	case SDLK_KP_0: return GK_NUMPAD0;
	case SDLK_KP_1: return GK_NUMPAD1;
	case SDLK_KP_2: return GK_NUMPAD2;
	case SDLK_KP_3: return GK_NUMPAD3;
	case SDLK_KP_4: return GK_NUMPAD4;
	case SDLK_KP_5: return GK_NUMPAD5;
	case SDLK_KP_6: return GK_NUMPAD6;
	case SDLK_KP_7: return GK_NUMPAD7;
	case SDLK_KP_8: return GK_NUMPAD8;
	case SDLK_KP_9: return GK_NUMPAD9;
	case SDLK_KP_DECIMAL: return GK_DECIMAL;
	case SDLK_KP_DIVIDE: return GK_DIVIDE;
	case SDLK_KP_MULTIPLY: return GK_MULTIPLY;
	case SDLK_KP_MINUS: return GK_SUBTRACT;
	case SDLK_KP_PLUS: return GK_ADD;
	case SDLK_KP_ENTER: return GK_RETURN;
	case SDLK_LSHIFT: return GK_SHIFT;
	case SDLK_LCTRL: return GK_CONTROL;
	case SDLK_LALT: return GK_MENU;
	case SDLK_LGUI: return GK_LWIN;
	case SDLK_RSHIFT: return GK_SHIFT;
	case SDLK_RCTRL: return GK_CONTROL;
	case SDLK_RALT: return GK_MENU;
	case SDLK_RGUI: return GK_RWIN;
	case SDLK_MENU: return GK_UNKNOWN;
	default: return GK_UNKNOWN;
	}
}
x
mu_int32 SDLScanCodeToUltralightKeyCode(SDL_Scancode key)
{
	using namespace ultralight::KeyCodes;

	switch (key) {
	case SDL_SCANCODE_SPACE: return GK_SPACE;
	case SDL_SCANCODE_APOSTROPHE: return GK_OEM_7;
	case SDL_SCANCODE_COMMA: return GK_OEM_COMMA;
	case SDL_SCANCODE_MINUS: return GK_OEM_MINUS;
	case SDL_SCANCODE_PERIOD: return GK_OEM_PERIOD;
	case SDL_SCANCODE_SLASH: return GK_OEM_2;
	case SDL_SCANCODE_0: return GK_0;
	case SDL_SCANCODE_1: return GK_1;
	case SDL_SCANCODE_2: return GK_2;
	case SDL_SCANCODE_3: return GK_3;
	case SDL_SCANCODE_4: return GK_4;
	case SDL_SCANCODE_5: return GK_5;
	case SDL_SCANCODE_6: return GK_6;
	case SDL_SCANCODE_7: return GK_7;
	case SDL_SCANCODE_8: return GK_8;
	case SDL_SCANCODE_9: return GK_9;
	case SDL_SCANCODE_SEMICOLON: return GK_OEM_1;
	case SDL_SCANCODE_EQUALS: return GK_OEM_PLUS;
	case SDL_SCANCODE_A: return GK_A;
	case SDL_SCANCODE_B: return GK_B;
	case SDL_SCANCODE_C: return GK_C;
	case SDL_SCANCODE_D: return GK_D;
	case SDL_SCANCODE_E: return GK_E;
	case SDL_SCANCODE_F: return GK_F;
	case SDL_SCANCODE_G: return GK_G;
	case SDL_SCANCODE_H: return GK_H;
	case SDL_SCANCODE_I: return GK_I;
	case SDL_SCANCODE_J: return GK_J;
	case SDL_SCANCODE_K: return GK_K;
	case SDL_SCANCODE_L: return GK_L;
	case SDL_SCANCODE_M: return GK_M;
	case SDL_SCANCODE_N: return GK_N;
	case SDL_SCANCODE_O: return GK_O;
	case SDL_SCANCODE_P: return GK_P;
	case SDL_SCANCODE_Q: return GK_Q;
	case SDL_SCANCODE_R: return GK_R;
	case SDL_SCANCODE_S: return GK_S;
	case SDL_SCANCODE_T: return GK_T;
	case SDL_SCANCODE_U: return GK_U;
	case SDL_SCANCODE_V: return GK_V;
	case SDL_SCANCODE_W: return GK_W;
	case SDL_SCANCODE_X: return GK_X;
	case SDL_SCANCODE_Y: return GK_Y;
	case SDL_SCANCODE_Z: return GK_Z;
	case SDL_SCANCODE_LEFTBRACKET: return GK_OEM_4;
	case SDL_SCANCODE_BACKSLASH: return GK_OEM_5;
	case SDL_SCANCODE_RIGHTBRACKET: return GK_OEM_6;
	case SDL_SCANCODE_GRAVE: return GK_OEM_3;
	case SDL_SCANCODE_ESCAPE: return GK_ESCAPE;
	case SDL_SCANCODE_RETURN: return GK_RETURN;
	case SDL_SCANCODE_TAB: return GK_TAB;
	case SDL_SCANCODE_BACKSPACE: return GK_BACK;
	case SDL_SCANCODE_INSERT: return GK_INSERT;
	case SDL_SCANCODE_DELETE: return GK_DELETE;
	case SDL_SCANCODE_RIGHT: return GK_RIGHT;
	case SDL_SCANCODE_LEFT: return GK_LEFT;
	case SDL_SCANCODE_DOWN: return GK_DOWN;
	case SDL_SCANCODE_UP: return GK_UP;
	case SDL_SCANCODE_PAGEUP: return GK_PRIOR;
	case SDL_SCANCODE_PAGEDOWN: return GK_NEXT;
	case SDL_SCANCODE_HOME: return GK_HOME;
	case SDL_SCANCODE_END: return GK_END;
	case SDL_SCANCODE_CAPSLOCK: return GK_CAPITAL;
	case SDL_SCANCODE_SCROLLLOCK: return GK_SCROLL;
	case SDL_SCANCODE_NUMLOCKCLEAR: return GK_NUMLOCK;
	case SDL_SCANCODE_PRINTSCREEN: return GK_SNAPSHOT;
	case SDL_SCANCODE_PAUSE: return GK_PAUSE;
	case SDL_SCANCODE_F1: return GK_F1;
	case SDL_SCANCODE_F2: return GK_F2;
	case SDL_SCANCODE_F3: return GK_F3;
	case SDL_SCANCODE_F4: return GK_F4;
	case SDL_SCANCODE_F5: return GK_F5;
	case SDL_SCANCODE_F6: return GK_F6;
	case SDL_SCANCODE_F7: return GK_F7;
	case SDL_SCANCODE_F8: return GK_F8;
	case SDL_SCANCODE_F9: return GK_F9;
	case SDL_SCANCODE_F10: return GK_F10;
	case SDL_SCANCODE_F11: return GK_F11;
	case SDL_SCANCODE_F12: return GK_F12;
	case SDL_SCANCODE_F13: return GK_F13;
	case SDL_SCANCODE_F14: return GK_F14;
	case SDL_SCANCODE_F15: return GK_F15;
	case SDL_SCANCODE_F16: return GK_F16;
	case SDL_SCANCODE_F17: return GK_F17;
	case SDL_SCANCODE_F18: return GK_F18;
	case SDL_SCANCODE_F19: return GK_F19;
	case SDL_SCANCODE_F20: return GK_F20;
	case SDL_SCANCODE_F21: return GK_F21;
	case SDL_SCANCODE_F22: return GK_F22;
	case SDL_SCANCODE_F23: return GK_F23;
	case SDL_SCANCODE_F24: return GK_F24;
	case SDL_SCANCODE_KP_0: return GK_NUMPAD0;
	case SDL_SCANCODE_KP_1: return GK_NUMPAD1;
	case SDL_SCANCODE_KP_2: return GK_NUMPAD2;
	case SDL_SCANCODE_KP_3: return GK_NUMPAD3;
	case SDL_SCANCODE_KP_4: return GK_NUMPAD4;
	case SDL_SCANCODE_KP_5: return GK_NUMPAD5;
	case SDL_SCANCODE_KP_6: return GK_NUMPAD6;
	case SDL_SCANCODE_KP_7: return GK_NUMPAD7;
	case SDL_SCANCODE_KP_8: return GK_NUMPAD8;
	case SDL_SCANCODE_KP_9: return GK_NUMPAD9;
	case SDL_SCANCODE_KP_DECIMAL: return GK_DECIMAL;
	case SDL_SCANCODE_KP_DIVIDE: return GK_DIVIDE;
	case SDL_SCANCODE_KP_MULTIPLY: return GK_MULTIPLY;
	case SDL_SCANCODE_KP_MINUS: return GK_SUBTRACT;
	case SDL_SCANCODE_KP_PLUS: return GK_ADD;
	case SDL_SCANCODE_KP_ENTER: return GK_RETURN;
	case SDL_SCANCODE_LSHIFT: return GK_SHIFT;
	case SDL_SCANCODE_LCTRL: return GK_CONTROL;
	case SDL_SCANCODE_LALT: return GK_MENU;
	case SDL_SCANCODE_LGUI: return GK_LWIN;
	case SDL_SCANCODE_RSHIFT: return GK_SHIFT;
	case SDL_SCANCODE_RCTRL: return GK_CONTROL;
	case SDL_SCANCODE_RALT: return GK_MENU;
	case SDL_SCANCODE_RGUI: return GK_RWIN;
	case SDL_SCANCODE_MENU: return GK_UNKNOWN;
	default: return GK_UNKNOWN;
	}
}

const mu_boolean SDLIsKeypad(const SDL_Event *event)
{
	switch (event->key.keysym.sym)
	{
	case SDL_SCANCODE_KP_DIVIDE:
	case SDL_SCANCODE_KP_MULTIPLY:
	case SDL_SCANCODE_KP_MINUS:
	case SDL_SCANCODE_KP_PLUS:
	case SDL_SCANCODE_KP_ENTER:
	case SDL_SCANCODE_KP_1:
	case SDL_SCANCODE_KP_2:
	case SDL_SCANCODE_KP_3:
	case SDL_SCANCODE_KP_4:
	case SDL_SCANCODE_KP_5:
	case SDL_SCANCODE_KP_6:
	case SDL_SCANCODE_KP_7:
	case SDL_SCANCODE_KP_8:
	case SDL_SCANCODE_KP_9:
	case SDL_SCANCODE_KP_0:
	case SDL_SCANCODE_KP_PERIOD:
		return true;

	case SDL_SCANCODE_KP_EQUALS:
		return true;

	case SDL_SCANCODE_KP_COMMA:
	case SDL_SCANCODE_KP_EQUALSAS400:

	case SDL_SCANCODE_KP_00:
	case SDL_SCANCODE_KP_000:
	case SDL_SCANCODE_KP_LEFTPAREN:
	case SDL_SCANCODE_KP_RIGHTPAREN:
	case SDL_SCANCODE_KP_LEFTBRACE:
	case SDL_SCANCODE_KP_RIGHTBRACE:
	case SDL_SCANCODE_KP_TAB:
	case SDL_SCANCODE_KP_BACKSPACE:
	case SDL_SCANCODE_KP_A:
	case SDL_SCANCODE_KP_B:
	case SDL_SCANCODE_KP_C:
	case SDL_SCANCODE_KP_D:
	case SDL_SCANCODE_KP_E:
	case SDL_SCANCODE_KP_F:
	case SDL_SCANCODE_KP_XOR:
	case SDL_SCANCODE_KP_POWER:
	case SDL_SCANCODE_KP_PERCENT:
	case SDL_SCANCODE_KP_LESS:
	case SDL_SCANCODE_KP_GREATER:
	case SDL_SCANCODE_KP_AMPERSAND:
	case SDL_SCANCODE_KP_DBLAMPERSAND:
	case SDL_SCANCODE_KP_VERTICALBAR:
	case SDL_SCANCODE_KP_DBLVERTICALBAR:
	case SDL_SCANCODE_KP_COLON:
	case SDL_SCANCODE_KP_HASH:
	case SDL_SCANCODE_KP_SPACE:
	case SDL_SCANCODE_KP_AT:
	case SDL_SCANCODE_KP_EXCLAM:
	case SDL_SCANCODE_KP_MEMSTORE:
	case SDL_SCANCODE_KP_MEMRECALL:
	case SDL_SCANCODE_KP_MEMCLEAR:
	case SDL_SCANCODE_KP_MEMADD:
	case SDL_SCANCODE_KP_MEMSUBTRACT:
	case SDL_SCANCODE_KP_MEMMULTIPLY:
	case SDL_SCANCODE_KP_MEMDIVIDE:
	case SDL_SCANCODE_KP_PLUSMINUS:
	case SDL_SCANCODE_KP_CLEAR:
	case SDL_SCANCODE_KP_CLEARENTRY:
	case SDL_SCANCODE_KP_BINARY:
	case SDL_SCANCODE_KP_OCTAL:
	case SDL_SCANCODE_KP_DECIMAL:
	case SDL_SCANCODE_KP_HEXADECIMAL:
		return true;
	}

	return false;
}
#endif