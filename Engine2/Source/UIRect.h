#pragma once
#include "UIElement.h"
namespace E2
{
    class UIRect : public UIElement
    {
    protected:
        Color m_color;
        bool m_drawOutline;
        bool m_alwaysDraw;
    public:
        UIRect();
        UIRect(Color color, bool alwaysDraw, bool drawOutline);
        virtual void Draw()override;
        virtual void Update()override;
    };
}