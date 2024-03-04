#ifndef __UI_NOESISGUI_SDL_H__
#define __UI_NOESISGUI_SDL_H__

#pragma once

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
Noesis::Key SDLKeyCodeToNoesisKeyCode(SDL_Keycode key);
#endif

#endif
