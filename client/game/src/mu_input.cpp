#include "mu_precompiled.h"
#include "mu_input.h"

namespace MUInput
{
	glm::ivec2 MousePosition;
	mu_float MouseWheel = 0.0f;
	mu_uint8 MouseState[MOUSE_BUTTON_MAX] = {};
	mu_uint8 MouseOldState[MOUSE_BUTTON_MAX] = {};
	mu_uint8 MouseFastCmp[MOUSE_BUTTON_MAX] = {};

	mu_boolean KeyState[KEYBOARD_KEY_SIZE] = {};
	mu_boolean KeyOldState[KEYBOARD_KEY_SIZE] = {};
	mu_boolean KeyFastCmp[KEYBOARD_KEY_SIZE] = {};

	void ShowCursor(mu_boolean show)
	{
		SDL_ShowCursor(show);
	}

	void Reset()
	{
		MouseWheel = 0.0f;
		mu_zeromem(MouseState, sizeof(MouseState));
		mu_zeromem(MouseOldState, sizeof(MouseOldState));
		mu_zeromem(KeyState, sizeof(KeyState));
		mu_zeromem(KeyOldState, sizeof(KeyOldState));
	}

	void ProcessKeys()
	{
		MouseWheel = 0.0f;
		mu_memcpy(KeyOldState, KeyState, sizeof(KeyState));
		mu_memcpy(MouseOldState, MouseState, sizeof(MouseState));
	}

	void SetMousePosition(mu_int32 x, mu_int32 y)
	{
		MousePosition.x = x;
		MousePosition.y = y;
	}

	const glm::ivec2 GetMousePosition()
	{
		return MousePosition;
	}

	void SetMouseButton(mu_uint32 index, mu_uint8 state)
	{
		MouseState[index] = state;
	}

	const mu_boolean IsMousePressed(mu_int32 index)
	{
		return MouseState[index] == MOUSE_STATE_CLICK && MouseOldState[index] == MOUSE_STATE_NONE;
	}

	const mu_boolean IsMouseDoublePressed(mu_int32 index)
	{
		return MouseState[index] == MOUSE_STATE_DOUBLECLICK;
	}

	const mu_boolean IsMousePressing(mu_uint32 index)
	{
		return MouseState[index] != MOUSE_STATE_NONE;
	}

	const mu_boolean IsAnyMousePressing()
	{
		return mu_memcmp(MouseFastCmp, MouseState, sizeof(MouseFastCmp)) != 0;
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
		KeyState[Key] = true;
		KeyOldState[Key] = false;
	}

	void SetKeyUp(mu_uint32 Key)
	{
		KeyState[Key] = false;
	}

	mu_boolean GetKeyState(mu_uint32 Key)
	{
		return KeyState[Key];
	}

	const mu_boolean IsKeyPressed(const mu_uint32 key)
	{
		return KeyState[key] == true && KeyOldState[key] == true;
	}

	const mu_boolean IsKeyPressing(const mu_uint32 key)
	{
		return KeyState[key] == true;
	}

	const mu_boolean IsAnyKeyPressing()
	{
		return mu_memcmp(KeyFastCmp, KeyState, sizeof(KeyFastCmp)) != 0;
	}

	const mu_boolean IsShiftPressing()
	{
		return IsKeyPressing(SDL_SCANCODE_LSHIFT) == true || IsKeyPressing(SDL_SCANCODE_RSHIFT) == true;
	}
};