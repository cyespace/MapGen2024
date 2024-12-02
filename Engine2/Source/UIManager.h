#pragma once
#include <vector>
namespace E2
{
    class UIElement;
    class UIManager
    {
    private:
        //UIElement* m_decoy;
        bool m_drawDebugFrame;
        UIElement* m_pLastHit;
        UIElement* m_pMousePressed;
        UIElement* m_pKeyFocus;
        std::vector<UIElement*> m_rootElements;
    public:
        UIManager();
        ~UIManager();
        void AddElement(UIElement* pElement);
        void Update();
        void Draw();
        UIElement* HitTest();
        void WillDrawDebugFrame(bool b) { m_drawDebugFrame = b; }
        void ClearUI();
        UIElement* GetElement(const char* pName);
    };
}