#pragma once
#include <array>
#include "Vector2.h"

namespace E2
{
    class Mouse
    {
    public:
        enum class Button
        {
            Left,
            Middle,
            Right,
            ButtonMax,
        };

    private:
        std::array<bool, (uint32_t)Button::ButtonMax> m_mouseState;
        std::array<bool, (uint32_t)Button::ButtonMax> m_previousMouseState;
        bool m_isAnyButtonPressed;
        Vector2 m_pos;
    public:
        Mouse()
            :m_mouseState{ false }
            , m_previousMouseState{ false }
            , m_isAnyButtonPressed{ false }
            , m_pos{0,0}
        {}
        ~Mouse() = default;

        void SetMousePos(Vector2 pos) { m_pos = pos; }
        Vector2 GetMousePos()const { return m_pos; }

        void SetState(Button button, bool state)
        {
            if ((uint32_t)button >= m_mouseState.size())
            {
                return;
            }
            m_mouseState[(uint32_t)button] = state;
            if(state)
                m_isAnyButtonPressed = true;
        }

        bool IsButtonDown(Button button)const { return m_mouseState[(uint32_t)button]; }
        bool IsButtonPressed(Button button)const
        {
            return m_mouseState[(uint32_t)button] && !m_previousMouseState[(uint32_t)button];
        }
        bool IsButtonReleased(Button button)const
        {
            return !m_mouseState[(uint32_t)button] && m_previousMouseState[((uint32_t)button)];
        }
        bool IsAnyButtonPressed() const { return m_isAnyButtonPressed; }

        void Reset() { m_mouseState = { false }; m_previousMouseState = { false }; }

        void NextFrame() { m_previousMouseState = m_mouseState; m_isAnyButtonPressed = false; }
    };
}