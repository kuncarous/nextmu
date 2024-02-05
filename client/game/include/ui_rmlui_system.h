#ifndef __UI_RMLUI_SYSTEM_H__
#define __UI_RMLUI_SYSTEM_H__

#pragma once

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_RMLUI
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/Types.h>

struct SDL_Window;
struct SDL_Cursor;

namespace UIRmlUI
{
    class NSystemInterface : public Rml::SystemInterface
    {
    public:
        NSystemInterface();
        ~NSystemInterface();

        // Optionally, provide or change the window to be used for setting the mouse cursors.
        void SetWindow(SDL_Window* window);

        // -- Inherited from Rml::SystemInterface  --

        double GetElapsedTime() override;

        void SetMouseCursor(const Rml::String& cursor_name) override;

        void SetClipboardText(const Rml::String& text) override;
        void GetClipboardText(Rml::String& text) override;

    private:
        SDL_Window* window = nullptr;

        SDL_Cursor* cursor_default = nullptr;
        SDL_Cursor* cursor_move = nullptr;
        SDL_Cursor* cursor_pointer = nullptr;
        SDL_Cursor* cursor_resize = nullptr;
        SDL_Cursor* cursor_cross = nullptr;
        SDL_Cursor* cursor_text = nullptr;
        SDL_Cursor* cursor_unavailable = nullptr;
    };

    // Applies input on the context based on the given SDL event.
    // @return True if the event is still propagating, false if it was handled by the context.
    mu_boolean InputEventHandler(Rml::Context* context, SDL_Event& ev);

    // Converts the SDL key to RmlUi key.
    Rml::Input::KeyIdentifier ConvertKey(mu_int32 sdl_key);

    // Converts the SDL mouse button to RmlUi mouse button.
    mu_int32 ConvertMouseButton(mu_int32 sdl_mouse_button);

    // Returns the active RmlUi key modifier state.
    mu_int32 GetKeyModifierState();
}
#endif

#endif
