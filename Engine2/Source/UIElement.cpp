#include "pch.h"
#include "UIElement.h"
#include "Engine.h"
#include <cmath>

E2::UIElement::UIElement()
    : m_designX{}
    , m_designY{}
    , m_designW{ UICoordType::Percentage,1.f }
    , m_designH{ UICoordType::Percentage,1.f }
    , m_designOriginX{}
    , m_designOriginY{}
    , m_realRegion{ 0,0,0,0 }
    , m_childLayout{ LayoutMode::None }
    , m_debugMouseOverThis{ false }
    , m_isVisable{ true }
    , m_pParent{nullptr}
{
}

E2::UIElement::~UIElement()
{
    for (auto pChild : m_children)
    {
        delete pChild;
    }
    m_children.clear();
}

void E2::UIElement::Reposition(int regionX, int regionY, int regionW, int regionH)
{
    m_realRegion.w = (int)m_designW.Resolve((float)regionW);
    m_realRegion.h = (int)m_designH.Resolve((float)regionH);
    int originX = (int)m_designOriginX.Resolve((float)m_realRegion.w);
    int originY = (int)m_designOriginY.Resolve((float)m_realRegion.h);
    m_realRegion.x = (int)(m_designX.Resolve((float)regionW)) + regionX - originX;
    m_realRegion.y = (int)(m_designY.Resolve((float)regionH)) + regionY - originY;

    uint32_t nextX = 0;
    uint32_t nextY = 0;
    for (auto pChild : m_children)
    {
        switch (m_childLayout)
        {
        case LayoutMode::StackHorizontal:
        {
            pChild->Reposition(m_realRegion.x + nextX, m_realRegion.y, m_realRegion.w, m_realRegion.h);
            nextX += pChild->GetActualW();
            break;
        }
        case LayoutMode::StackVertical:
        {
            pChild->Reposition(m_realRegion.x, m_realRegion.y + nextY, m_realRegion.w, m_realRegion.h);
            nextY += pChild->GetActualH();
            break;
        }
        case LayoutMode::WrapHorizontal:
        {
            pChild->Reposition(m_realRegion.x + nextX, m_realRegion.y + nextY, m_realRegion.w, m_realRegion.h);
            if (pChild->GetActualX() + pChild->GetActualW() > m_realRegion.w)
            {
                nextX = 0;
                nextY += pChild->GetActualH();
                pChild->Reposition(m_realRegion.x, m_realRegion.y + nextY, m_realRegion.w, m_realRegion.h);
            }
            nextX += pChild->GetActualW();
            break;
        }
        case LayoutMode::WrapVertical:
        {
            pChild->Reposition(m_realRegion.x + nextX, m_realRegion.y + nextY, m_realRegion.w, m_realRegion.h);
            if (pChild->GetActualY() + pChild->GetActualH() > m_realRegion.h)
            {
                nextY = 0;
                nextX += pChild->GetActualW();
                pChild->Reposition(m_realRegion.x + nextX, m_realRegion.y, m_realRegion.w, m_realRegion.h);
            }
            nextY += pChild->GetActualH();
            break;
        }
        case LayoutMode::None:
        {
            pChild->Reposition(m_realRegion.x, m_realRegion.y, m_realRegion.w, m_realRegion.h);
            break;
        }
        }
    }
}

void E2::UIElement::SetPositionInfo(UIElementData& data)
{
    auto GetDesignCoord = [](float number)->UIDesignCoord
    {
        UICoordType coorType = number < 1.0 ? UICoordType::Percentage : UICoordType::Pixels;

        return { coorType , number };
    };

    SetPosition(GetDesignCoord(data.posX), GetDesignCoord(data.posY));
    SetOrigin(GetDesignCoord(data.originX), GetDesignCoord(data.originY));
    SetDimension(GetDesignCoord(data.dimensionX), GetDesignCoord(data.dimensionY));
}

int E2::UIElement::GetWindowWidth()
{
    return Engine::Get().GetWindowSize().x;
}

int E2::UIElement::GetWindowHeight()
{
    return Engine::Get().GetWindowSize().y;
}

E2::UIElement* E2::UIElement::HitTest(Vector2 pos)
{
    if (!Engine::Get().PointInRect(pos.x, pos.y, m_realRegion))
    {
        return nullptr;
    }
    
    for (auto element : m_children)
    {
        auto hit = element->HitTest(pos);
        if (hit)
        {
            return hit;
        }
    }
    return this;
}
