#pragma once
#include <array>

namespace E2
{
    class Keyboard
    {
    public:
        enum class Key
        {
            A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
            Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0,
            Space, Return, BackSpace, Escape,
            Left, Right, Up, Down,
            LeftCtrl, LeftAlt, LeftShift,
            F1,F2,F3,
            KeyCodeMax,
        };

    private:
        std::array<bool, (uint32_t)Key::KeyCodeMax> m_KeyboardState;
        std::array<bool, (uint32_t)Key::KeyCodeMax> m_PreviousKeyboardState;
        Key m_lastKey;
        bool m_isAnykeyPressed;
    public:
        Keyboard()
            : m_KeyboardState{ false }
            , m_PreviousKeyboardState{ false }
            , m_isAnykeyPressed{ false }
            , m_lastKey{ Key:: KeyCodeMax }
        {}
        ~Keyboard() = default;
        void SetState(Key key, bool state)
        {
            if ((uint32_t)key >= m_KeyboardState.size())
            {
                return;
            }
            m_KeyboardState[(uint32_t)key] = state;
            if (state)
            {
                m_lastKey = key;
                m_isAnykeyPressed = true;
            }
        }
        bool IsKeyDown(Key key)const { return m_KeyboardState[(uint32_t)key]; }
        bool IsKeyPressed(Key key)const
        {
            return m_KeyboardState[(uint32_t)key] && !m_PreviousKeyboardState[(uint32_t)key];
        }

        // equivalent to WasKeyDown()
        bool IsKeyReleased(Key key)const
        {
            return !m_KeyboardState[(uint32_t)key] && m_PreviousKeyboardState[((uint32_t)key)];
        }

        bool IsAnykeyPressed() const { return m_isAnykeyPressed; }
        Key GetLastKeyPressed() const { return m_lastKey; }
        void Reset() { m_KeyboardState = { false }; m_PreviousKeyboardState = { false }; }

        void NextFrame() { m_PreviousKeyboardState = m_KeyboardState; m_isAnykeyPressed = false; }
    };
}