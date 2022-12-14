#ifndef __UI_ULTRALIGHT_SDL_H__
#define __UI_ULTRALIGHT_SDL_H__

#pragma once

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_ULTRALIGHT
mu_int32 SDLModsToUltralightMods(mu_uint16 mods);
mu_int32 SDLKeyCodeToUltralightKeyCode(SDL_Keycode key);
mu_int32 SDLScanCodeToUltralightKeyCode(SDL_Scancode key);
const mu_boolean SDLIsKeypad(const SDL_Event *event);
#endif

#endif