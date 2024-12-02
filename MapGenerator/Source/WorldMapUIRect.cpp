#include "WorldMapUIRect.h"
#include "GlobalFunctions.h"
#include "SimpleMath.h"

#include <CallBack.h>

WorldMapUIRect::WorldMapUIRect(E2::Rect restrictRect ,const E2::Color& color)
    : UIRect(color,false,true)
    , m_restrict{ restrictRect }
{
}

WorldMapUIRect::~WorldMapUIRect()
{
    delete m_pCallBack;
    m_pCallBack = nullptr;
}

void WorldMapUIRect::Update()
{
    if (!m_isVisable)
    {
        return;
    }
    auto mousePos = GetEngine().GetMousePos();
    m_realRegion.x = mousePos.x - m_realRegion.w / 2;
    m_realRegion.y = mousePos.y - m_realRegion.h / 2;

    m_realRegion.x = E2::Clamp<int>(m_restrict.x , (m_restrict.x + m_restrict.w) - m_realRegion.w, m_realRegion.x);
    m_realRegion.y = E2::Clamp<int>(m_restrict.y , (m_restrict.y + m_restrict.h) - m_realRegion.h, m_realRegion.y);
}

void WorldMapUIRect::OnNotify(E2::Event evt)
{
    if (!m_isVisable)
    {
        return;
    }
    else
    {
        if (this == HitTest({ evt.m_mouseEvent.x, evt.m_mouseEvent.y }))
        {
            m_pCallBack->Call();
        }
    }
}

void WorldMapUIRect::SetCallBack(E2::CallBack* pCallBack)
{
    m_pCallBack = std::move(pCallBack);
}

std::tuple<float, float, float, float> WorldMapUIRect::GetSelectionRelativePosition()
{
    auto& myRect = GetRealRegion();
    float xf = (float)(myRect.x - m_restrict.x) / (float)m_restrict.w;
    float yf = (float)(myRect.y - m_restrict.y) / (float)m_restrict.h;
    float wf = (float)myRect.w / (float)m_restrict.w;
    float hf = (float)myRect.h / (float)m_restrict.h;

    return std::make_tuple(xf,yf,wf,hf);
}
