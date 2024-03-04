#ifndef __MU_INPUT_H__
#define __MU_INPUT_H__

#pragma once

enum
{
	MOUSE_STATE_NONE = 0,
	MOUSE_STATE_CLICK = 1,
	MOUSE_STATE_DOUBLECLICK = 2,

	MOUSE_USED_NONE = 0,
	MOUSE_USED_PRESSED = 1,
	MOUSE_USED_ALL = 2,
};

enum
{
	KEYBOARD_KEY_SIZE = 1024,
	KEYBOARD_KEY_PRESSED = 0x80,

	MOUSE_BUTTON_LEFT = 0,
	MOUSE_BUTTON_MIDDLE = 1,
	MOUSE_BUTTON_RIGHT = 2,
	MOUSE_BUTTON_MAX,

	MOUSE_FLAG_PRESSED = 0,
	MOUSE_FLAG_DOUBLECLICK,
	MOUSE_FLAG_PRESSING,
	MOUSE_FLAG_RELEASED,
	MOUSE_FLAGS,

	MOUSE_FLAG_LEFT = 1,
	MOUSE_FLAG_MIDDLE = 2,
	MOUSE_FLAG_RIGHT = 4,
};

enum
{
	VFLAG_LBUTTON = (1 << 0),
	VFLAG_RBUTTON = (1 << 1),
	VFLAG_SHIFT = (1 << 2),
	VFLAG_CONTROL = (1 << 3),
	VFLAG_MBUTTON = (1 << 4),
	VFLAG_XBUTTON1 = (1 << 5),
	VFLAG_XBUTTON2 = (1 << 6),
};

namespace MUInput
{
	void ShowCursor(mu_boolean show);
	void Reset();
	void ProcessKeys();

	void SetMousePosition(mu_int32 x, mu_int32 y);
	const glm::ivec2 GetMousePosition();

	void SetMouseButton(mu_uint32 index, mu_uint8 state);
	const mu_boolean IsMousePressed(mu_int32 index);
	const mu_boolean IsMouseDoublePressed(mu_int32 index);
	const mu_boolean IsMousePressing(mu_uint32 index);
	const mu_boolean IsAnyMousePressing();

	void AddMouseWheel(mu_int32 wheel);
	const mu_float GetMouseWheel();

	void SetKeyDown(mu_uint32 Key);
	void SetKeyUp(mu_uint32 Key);
	mu_boolean GetKeyState(mu_uint32 Key);
	const mu_boolean IsKeyPressing(const mu_uint32 key);
	const mu_boolean IsAnyKeyPressing();
	const mu_boolean IsShiftPressing();

	template<typename T>
	NEXTMU_INLINE const T GetRealPixelSize(const T in)
	{
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS
		return static_cast<T>(static_cast<mu_float>(in) * ScreenScaleRatio);
#else
		return in;
#endif
	}
};

#endif