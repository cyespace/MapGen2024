#pragma once

#include <UIRect.h>
#include <Color.h>

#include <tuple>

namespace E2
{
    struct CallBack;
}

class WorldMapUIRect : public E2::UIRect
{
private:
    E2::Rect m_restrict{};
    E2::CallBack* m_pCallBack = nullptr;
public:
    WorldMapUIRect(E2::Rect restrictRect, const E2::Color& color);
    virtual ~WorldMapUIRect();
    virtual void OnRollOver() final {}
    virtual void Update() final;
    virtual void OnNotify(E2::Event evt);
    void SetCallBack(E2::CallBack* pCallBack);

    std::tuple<float, float, float, float> GetSelectionRelativePosition();
};