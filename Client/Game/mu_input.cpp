#include "stdafx.h"
#include "mu_input.h"

namespace MUInput
{
	mu_float MouseWheel = 0.0f;
	mu_uint8 MouseState[MOUSE_BUTTON_MAX] = {};
	mu_uint8 MouseUsed[MOUSE_BUTTON_MAX] = {};

	mu_boolean KeyPressed[KEYBOARD_KEY_SIZE] = {};
	mu_uint8 KeyState[KEYBOARD_KEY_SIZE] = {};

	void ShowCursor(mu_boolean show)
	{
		SDL_ShowCursor(show);
	}

	void Reset()
	{
		MouseWheel = 0.0f;
	}

	void ProcessKeys()
	{
		mu_memcpy(KeyState, KeyPressed, sizeof(KeyState));
	}

	void SetMouseButton(mu_uint32 index, mu_uint8 state)
	{
		MouseState[index] = state;
		MouseUsed[index] = MOUSE_USED_NONE;
	}

	const mu_boolean IsMousePressed(mu_int32 index)
	{
		return MouseState[index] == MOUSE_STATE_CLICK;
	}

	const mu_boolean IsMouseDoublePressed(mu_int32 index)
	{
		return MouseState[index] == MOUSE_STATE_DOUBLECLICK;
	}

	const mu_boolean IsMousePressing(mu_uint32 index)
	{
		return MouseState[index] != MOUSE_STATE_NONE;
	}

	void AddMouseWheel(mu_int32 wheel)
	{
		MouseWheel += static_cast<mu_float>(wheel);
	}

	const mu_float GetMouseWheel()
	{
		return MouseWheel;
	}

	void SetKeyDown(mu_uint32 Key)
	{
		KeyPressed[Key] = true;
		KeyState[Key] = 0;
	}

	void SetKeyUp(mu_uint32 Key)
	{
		KeyPressed[Key] = false;
	}

	mu_boolean GetKeyState(mu_uint32 Key)
	{
		return KeyPressed[Key];
	}

	const mu_boolean IsKeyPressing(const mu_uint32 key)
	{
		return KeyPressed[key] == true;
	}

	const mu_boolean IsShiftPressing()
	{
		return IsKeyPressing(SDL_SCANCODE_LSHIFT) == true || IsKeyPressing(SDL_SCANCODE_RSHIFT) == true;
	}
};