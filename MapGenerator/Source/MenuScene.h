#pragma once

#include "Scene.h"

namespace E2
{
    class UIElement;
}

class MenuScene : public Scene
{
private:

    // UI
    //E2::UIElement* m_pInputBox = nullptr;
    //E2::UIElement* m_pRandomSeedButton = nullptr;
    //E2::UIElement* m_pGenerateMapButton = nullptr;
    //E2::UIElement* m_pQuitButton = nullptr;

public:
    MenuScene() = default;
    virtual ~MenuScene() = default;
    virtual void Update(float deltaTime) override;
    virtual SceneId GetId() override { return SceneId::MenuScene; }
    virtual bool Init()override;
    virtual void End()override;

};