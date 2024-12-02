#pragma once

#include "pch.h"
#include "Color.h"
#include "Rect.h"
#include "Vector2.h"
#include "Keyboard.h"
#include "EventListener.h"

namespace E2
{
    struct UIElementData
    {
        float posX = 0;
        float posY = 0;
        float originX = 0;
        float originY = 0;
        float dimensionX = 0;
        float dimensionY = 0;
    };

    enum class LayoutMode
    {
        StackHorizontal,
        StackVertical,
        WrapHorizontal,
        WrapVertical,
        None,
    };

    enum class UICoordType
    {
        Pixels,
        Percentage,
    };

    struct UIDesignCoord
    {
        UICoordType m_type;
        float m_value;
    public:
        UIDesignCoord()
            : UIDesignCoord{ UICoordType::Pixels,0 }
        {
        }
        UIDesignCoord(UICoordType myType, float value)
            : m_type{ myType }
            , m_value{ value }
        {
        }
        float Resolve(float dimension) const
        {
            if (m_type == UICoordType::Pixels)
            {
                return m_value;
            }
            else
            {
                return m_value * dimension;
            }
        }
    };

    class UIElement : public EventListener
    {
    protected:
        UIDesignCoord m_designX;
        UIDesignCoord m_designY;
        UIDesignCoord m_designW;
        UIDesignCoord m_designH;
        UIDesignCoord m_designOriginX;
        UIDesignCoord m_designOriginY;
        Rect m_realRegion;
        LayoutMode m_childLayout;
        bool m_debugMouseOverThis;
        bool m_isVisable;

        UIElement* m_pParent;
        std::vector<UIElement*> m_children;

    public:
        UIElement();
        virtual ~UIElement();
        
        void SetPosition(UIDesignCoord posX, UIDesignCoord posY)
        {
            m_designX = posX;
            m_designY = posY;
        }

        void SetOrigin(UIDesignCoord originX, UIDesignCoord originY)
        {
            m_designOriginX = originX;
            m_designOriginY = originY;
        }

        void SetDimension(UIDesignCoord width, UIDesignCoord height)
        {
            m_designW = width;
            m_designH = height;
        }
        void SetLayoutMode(LayoutMode mode) { m_childLayout = mode; }
        int GetActualX() const { return m_realRegion.x; }
        int GetActualY() const { return m_realRegion.y; }
        int GetActualW() const { return m_realRegion.w; }
        int GetActualH() const { return m_realRegion.h; }
        Rect& GetRealRegion(){ return m_realRegion; }
        void Reposition(int regionX, int regionY, int regionW, int regionH);
        void SetPositionInfo(UIElementData& data);

        void AddChild(UIElement* pChild) { pChild->m_pParent = this; m_children.push_back(pChild); }
        UIElement* GetParent() { return m_pParent; }
        static int GetWindowWidth();
        static int GetWindowHeight();

        bool GetMouseOverThis()const { return m_debugMouseOverThis; }

        UIElement* HitTest(Vector2 pos);

        virtual void Draw()
        {
            for (auto& child : m_children)
            {
                child->Draw();
            }
        }

        void SetVisable(bool visable) { m_isVisable = visable; }
        bool IsVisable() const { return m_isVisable; }
        virtual void DrawOutline(){}
        virtual void Update(){}
        virtual bool SupportsKeyFocus() const { return false; }
        virtual void OnRollOver() { if (m_pParent)m_pParent->OnRollOver(); m_debugMouseOverThis = true;}
        virtual void OnRollOut() { if (m_pParent)m_pParent->OnRollOut(); m_debugMouseOverThis = false; }
        virtual void OnPress() { if (m_pParent)m_pParent->OnPress(); }
        virtual void OnClick() { if (m_pParent)m_pParent->OnClick(); }
        virtual void OnReleaseOutside(){ if (m_pParent) m_pParent->OnReleaseOutside(); }
        virtual void OnFocusGained() {}
        virtual void OnFocusLost() {}
        virtual void OnKeyDown(E2::Keyboard::Key key){}
        virtual void OnKeyUp(E2::Keyboard::Key key) {}
        virtual void OnText(const char c) {}
    };
}