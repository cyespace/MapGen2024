#pragma once

#include "UIElement.h"

#include <unordered_map>

namespace E2
{
    struct UIButtonData : public UIElementData
    {
        E2::Color upColor;
        E2::Color downColor;
        E2::Color hoverColor;
    };

    struct CallBack;

    class UIButton : public UIElement
    {
    public:
        enum class State
        {
            Up,
            Over,
            Down,
            Selected,
        };
    private:
        bool m_isSelected;
        State m_currentState;
        std::unordered_map<State, UIElement*> m_stateElements;
        void(*m_pOnClickCallBack)();
        CallBack* m_pCallBack;

    public:
        UIButton(const Color& upColor, const Color& downColor, const Color& overColor);
        virtual ~UIButton();

        virtual void OnRollOver() override;
        virtual void OnRollOut() override;
        virtual void OnPress() override;
        virtual void OnClick() override;
        virtual void OnReleaseOutside() override;
        virtual void OnNotify(Event evt) override;

        void AddStateElement(State state, UIElement* pElement);
        void SetSelected(bool select);
        void ChangeState(State state);
        void SetCallBack(void(*pCallBack)());
        void SetCallBack(CallBack* pCallBack);
    };
}