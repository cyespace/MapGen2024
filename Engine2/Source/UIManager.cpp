#include "pch.h"
#include "UIManager.h"
#include "UIElement.h"
#include "UITextInput.h"
#include "Engine.h"

E2::UIManager::UIManager()
    //: m_decoy{nullptr}
    : m_drawDebugFrame{false}
    , m_pLastHit{nullptr}
    , m_pMousePressed{nullptr}
    , m_pKeyFocus{nullptr}
{
}

E2::UIManager::~UIManager()
{
    ClearUI();
}

void E2::UIManager::AddElement(UIElement* pElement)
{
    m_rootElements.push_back(pElement);
}

void E2::UIManager::Update()
{
    E2::UIElement* pHit = HitTest();
    if (pHit != m_pLastHit)
    {
        if (m_pLastHit)
        {
            m_pLastHit->OnRollOut();
        }
        if (pHit)
        {
            pHit->OnRollOver();
        }
        m_pLastHit = pHit;
    }

    if (E2::Engine::Get().IsAnyMouseButtonPressed())
    {
        //std::cout << "Mouse pressed at " << GetEngine().GetMousePos().x << ' ' << GetEngine().GetMousePos().y << '\n';

        if (m_pLastHit)
        {
            m_pLastHit->OnPress();
            //m_pLastHit->OnClick();
        }


        m_pMousePressed = m_pLastHit;

        E2::UIElement* pFocusable = m_pLastHit;
        while (pFocusable != nullptr)
        {
            if (pFocusable->SupportsKeyFocus())
                break;

            pFocusable = pFocusable->GetParent();
        }

        if (pFocusable)
        {
            if (pFocusable != m_pKeyFocus)
            {
                m_pKeyFocus = pFocusable;
                m_pKeyFocus->OnFocusGained();
            }
        }
        else
        {
            if (m_pKeyFocus)
                m_pKeyFocus->OnFocusLost();
            m_pKeyFocus = nullptr;
        }
    }

    if (E2::Engine::Get().IsAnyKeyPressed())
    {
        if (m_pKeyFocus)
            m_pKeyFocus->OnKeyDown(E2::Engine::Get().GetLastKeyPressed());
    }

    for (auto* pElement : m_rootElements)
    {
        if (pElement->IsVisable())
        {
            pElement->Update();
        }
    }
}

void E2::UIManager::Draw()
{
    for (auto* pElement : m_rootElements)
    {
        if (pElement && pElement->IsVisable())
        {
            pElement->Draw();
        }
        if (m_drawDebugFrame)
        {
            E2::Engine::Get().DrawRectOutline(pElement->GetRealRegion(),E2::RedColor::kPink);
        }
    }
}

E2::UIElement* E2::UIManager::HitTest()
{
    UIElement* pOutHit = nullptr;
    auto mousePos = E2::Engine::Get().GetMousePos();
    for (auto* pElement : m_rootElements)
    {
        if (pElement->IsVisable())
        {
            UIElement* pHit = pElement->HitTest(mousePos);
            if (pHit)
            {
                pOutHit = pHit;
            }
        }
    }
    return pOutHit;
}

void E2::UIManager::ClearUI()
{
    m_pLastHit = nullptr;
    m_pMousePressed = nullptr;
    m_pKeyFocus = nullptr;

    for (auto* pElement : m_rootElements)
    {
        delete pElement;
        pElement = nullptr;
    }
    m_rootElements.clear();
}

//TODO: Fix this
E2::UIElement* E2::UIManager::GetElement(const char* pName)
{
    std::string name{pName};
    if (name == "UITextInput")
    {
        for (auto* p : m_rootElements)
        {
            if (dynamic_cast<UITextInput*>(p))
            {
                return p;
            }
        }
    }

    return nullptr;
}
