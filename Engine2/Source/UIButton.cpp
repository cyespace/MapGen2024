#include "pch.h"
#include "UIButton.h"
#include "UIRect.h"
#include "CallBack.h"

E2::UIButton::UIButton(const Color& upColor, const Color& downColor, const Color& overColor)
    : m_isSelected{false}
    , m_currentState{State::Up}
    , m_pOnClickCallBack{nullptr}
{
    {
        UIRect* box = new UIRect(upColor, true, false);
        box->SetDimension({ UICoordType::Percentage, 1 }, { UICoordType::Percentage, 1});
        AddStateElement(UIButton::State::Up, box);
    }
    {
        UIRect* box = new UIRect(overColor, true, false);
        box->SetDimension({ UICoordType::Percentage, 1 }, { UICoordType::Percentage, 1 });
        AddStateElement(UIButton::State::Over, box);
    }
    {
        UIRect* box = new UIRect(downColor, true, false);
        box->SetDimension({ UICoordType::Percentage, 1 }, { UICoordType::Percentage, 1 });
        AddStateElement(UIButton::State::Down, box);
    }
}
E2::UIButton::~UIButton()
{
    if (m_pCallBack)
    {
        delete m_pCallBack;
        m_pCallBack = nullptr;
    }
}
void E2::UIButton::OnRollOver()
{
    UIElement::OnRollOver();

    if (!m_isSelected)
    {
        ChangeState(State::Over);
    }
}
void E2::UIButton::OnRollOut()
{
    UIElement::OnRollOut();

    if (!m_isSelected)
        ChangeState(State::Up);
}
void E2::UIButton::OnPress()
{
    UIElement::OnPress();

    ChangeState(State::Down);
}
void E2::UIButton::OnClick()
{
    UIElement::OnClick();

    ChangeState(State::Over);

    if (m_pOnClickCallBack)
        m_pOnClickCallBack();

    if (m_pCallBack)
    {
        m_pCallBack->Call();
    }
}
void E2::UIButton::OnReleaseOutside()
{
    UIElement::OnReleaseOutside();
    if (!m_isSelected)
        ChangeState(State::Up);
}
void E2::UIButton::OnNotify(Event evt)
{
    auto* pElement = HitTest({ evt.m_mouseEvent.x, evt.m_mouseEvent.y });
    if (pElement)
    {
        pElement->OnClick();
    }
}
void E2::UIButton::AddStateElement(State state, UIElement* pElement)
{
    if (!pElement->GetParent())
        AddChild(pElement);

    m_stateElements[state] = pElement;

    // Determine the initial state of the element
    pElement->SetVisable(state == m_currentState);
}
void E2::UIButton::SetSelected(bool select)
{
    m_isSelected = select;

    if (m_isSelected)
        ChangeState(State::Selected);
    else
        ChangeState(State::Up);
}

void E2::UIButton::ChangeState(State state)
{
    auto iterA = m_stateElements.find(m_currentState);
    if (iterA != m_stateElements.end())
    {
        iterA->second->SetVisable(false);
    }

    m_currentState = state;

    auto iterB = m_stateElements.find(m_currentState);
    if (iterB != m_stateElements.end())
    {
        iterB->second-> SetVisable(true);
    }
}

void E2::UIButton::SetCallBack(void(*pCallBack)())
{
    m_pOnClickCallBack = pCallBack;
}

void E2::UIButton::SetCallBack(CallBack* pCallBack)
{
    m_pCallBack = std::move(pCallBack);
}
