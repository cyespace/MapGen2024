#include "pch.h"
#include "UIRect.h"
#include "Engine.h"

E2::UIRect::UIRect()
    : UIRect(Color{ 255,255,255 },false,false)
{
}

E2::UIRect::UIRect(Color color,bool alwaysDraw, bool drawOutline)
    : m_color{ color }
    , m_drawOutline{ drawOutline }
    , m_alwaysDraw{ alwaysDraw }
{
}

void E2::UIRect::Draw()
{
    if (!m_isVisable)
    {
        return;
    }

    if (m_alwaysDraw||m_debugMouseOverThis)
    {
        Engine::Get().DrawRect(m_realRegion, m_color);
    }
    else if(m_drawOutline)
    {
        Engine::Get().DrawRectOutline(m_realRegion, m_color);
    }
    
    for (auto p : m_children)
    {
        p->Draw();
    }
}


// ??
void E2::UIRect::Update()
{
    auto vec = Engine::Get().GetMousePos();
    HitTest(vec);
}