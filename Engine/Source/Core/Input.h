#pragma once

#include "Core/Defines.h"
#include "Core/Event.h"

#include <memory>
#include <utility>

#define KEY_DEF(name, code) KEYCODE_##name = code

namespace Cosmos
{
    class COSMOS_API Input
    {
    public:
        typedef enum Keycode : u32
        {
            // alphabet
            KEY_DEF(A, 0x41),
            KEY_DEF(B, 0x42),
            KEY_DEF(C, 0x43),
            KEY_DEF(D, 0x44),
            KEY_DEF(E, 0x45),
            KEY_DEF(F, 0x46),
            KEY_DEF(G, 0x47),
            KEY_DEF(H, 0x48),
            KEY_DEF(I, 0x49),
            KEY_DEF(J, 0x4A),
            KEY_DEF(K, 0x4B),
            KEY_DEF(L, 0x4C),
            KEY_DEF(M, 0x4D),
            KEY_DEF(N, 0x4E),
            KEY_DEF(O, 0x4F),
            KEY_DEF(P, 0x50),
            KEY_DEF(Q, 0x51),
            KEY_DEF(R, 0x52),
            KEY_DEF(S, 0x53),
            KEY_DEF(T, 0x54),
            KEY_DEF(U, 0x55),
            KEY_DEF(V, 0x56),
            KEY_DEF(W, 0x57),
            KEY_DEF(X, 0x58),
            KEY_DEF(Y, 0x59),
            KEY_DEF(Z, 0x5A),

            // numbers
            KEY_DEF(NUMPAD0, 0x60),
            KEY_DEF(NUMPAD1, 0x61),
            KEY_DEF(NUMPAD2, 0x62),
            KEY_DEF(NUMPAD3, 0x63),
            KEY_DEF(NUMPAD4, 0x64),
            KEY_DEF(NUMPAD5, 0x65),
            KEY_DEF(NUMPAD6, 0x66),
            KEY_DEF(NUMPAD7, 0x67),
            KEY_DEF(NUMPAD8, 0x68),
            KEY_DEF(NUMPAD9, 0x69),

            // specials
            KEY_DEF(SPACE, 0x20),
            KEY_DEF(PRIOR, 0x21),
            KEY_DEF(NEXT, 0x22),
            KEY_DEF(END, 0x23),
            KEY_DEF(HOME, 0x24),
            KEY_DEF(SELECT, 0x29),
            KEY_DEF(PRINT, 0x2A),
            KEY_DEF(EXECUTE, 0x2B),
            KEY_DEF(SNAPSHOT, 0x2C),
            KEY_DEF(INSERT, 0x2D),
            KEY_DEF(DELETE, 0x2E),
            KEY_DEF(HELP, 0x2F),
            KEY_DEF(CONVERT, 0x1C),
            KEY_DEF(NONCONVERT, 0x1D),
            KEY_DEF(ACCEPT, 0x1E),
            KEY_DEF(MODECHANGE, 0x1F),
            KEY_DEF(BACKSPACE, 0x08),
            KEY_DEF(ENTER, 0x0D),
            KEY_DEF(TAB, 0x09),
            KEY_DEF(SHIFT, 0x10),
            KEY_DEF(CONTROL, 0x11),
            KEY_DEF(PAUSE, 0x13),
            KEY_DEF(CAPITAL, 0x14),
            KEY_DEF(ESCAPE, 0x1B),
            KEY_DEF(LWIN, 0x5B),
            KEY_DEF(RWIN, 0x5C),
            KEY_DEF(APPS, 0x5D),
            KEY_DEF(SLEEP, 0x5F),
            KEY_DEF(NUMLOCK, 0x90),
            KEY_DEF(SCROLL, 0x91),
            KEY_DEF(LSHIFT, 0xA0),
            KEY_DEF(RSHIFT, 0xA1),
            KEY_DEF(LCONTROL, 0xA2),
            KEY_DEF(RCONTROL, 0xA3),
            KEY_DEF(LMENU, 0xA4),
            KEY_DEF(RMENU, 0xA5),

            // functions
            KEY_DEF(F1, 0x70),
            KEY_DEF(F2, 0x71),
            KEY_DEF(F3, 0x72),
            KEY_DEF(F4, 0x73),
            KEY_DEF(F5, 0x74),
            KEY_DEF(F6, 0x75),
            KEY_DEF(F7, 0x76),
            KEY_DEF(F8, 0x77),
            KEY_DEF(F9, 0x78),
            KEY_DEF(F10, 0x79),
            KEY_DEF(F11, 0x7A),
            KEY_DEF(F12, 0x7B),

            // symbols
            KEY_DEF(ADD, 0x6B),
            KEY_DEF(SEPARATOR, 0x6C),
            KEY_DEF(SUBTRACT, 0x6D),
            KEY_DEF(DECIMAL, 0x6E),
            KEY_DEF(DIVIDE, 0x6F),
            KEY_DEF(MULTIPLY, 0x6A),
            KEY_DEF(NUMPAD_EQUAL, 0x92),
            KEY_DEF(MINUS, 0xBD),
            KEY_DEF(PLUS, 0xBB),
            KEY_DEF(SEMICOLON, 0xBA),
            KEY_DEF(COMMA, 0xBC),
            KEY_DEF(PERIOD, 0xBE),
            KEY_DEF(SLASH, 0xBF),
            KEY_DEF(GRAVE, 0xC0),

            // arrows
            KEY_DEF(LEFT, 0x25),
            KEY_DEF(UP, 0x26),
            KEY_DEF(RIGHT, 0x27),
            KEY_DEF(DOWN, 0x28),

            KEYCODE_MAX_KEYS

        } Keycode;

        typedef enum Buttoncode : u32
        {
            BUTTON_LEFT,
            BUTTON_RIGHT,
            BUTTON_MIDDLE,

            BUTTONCODE_MAX_BUTTONS
        } Buttoncode;

        struct KeyboardInfo
        {
            bool Keys[KEYCODE_MAX_KEYS] = { false };
        };

        struct MouseInfo
        {
            bool Buttons[BUTTONCODE_MAX_BUTTONS] = { false };
            std::pair<i32, i32> MousePosition = { 0, 0 };
        };

    public:

        // creates a smart-ptr input
        static std::shared_ptr<Input> Create();

        // constructor
        Input();

        // destructor
        ~Input();

        // returns a reference to the singleton
        static Input& Get() { return *s_Input; };

    public:

        // updates the input system
        void Update(f32 timestep);

    public:

        // current time related
        bool IsKeyDown(Keycode key);
        bool IsKeyUp(Keycode key);
        bool IsButtonDown(Buttoncode button);
        bool IsButtonUp(Buttoncode button);
        std::pair<i32, i32> GetCurrentMousePosition();

        // last time related
        bool WasKeyDown(Keycode key);
        bool WasKeyUp(Keycode key);
        bool WasButtonDown(Buttoncode button);
        bool WasButtonUp(Buttoncode button);
        std::pair<i32, i32> GetLastMousePosition();

        // input handlers
        void HandleKey(Keycode key, bool pressed);
        void HandleButton(Buttoncode button, bool pressed);
        void HandleMove(i32 x, i32 y);
        void HandleWheel(i32 z);

    private:

        static Input* s_Input;
        KeyboardInfo m_LastKeyboardInfo;
        KeyboardInfo m_CurrentKeyboardInfo;
        MouseInfo m_LastMouseInfo;
        MouseInfo m_CurrentMouseInfo;
    };
}