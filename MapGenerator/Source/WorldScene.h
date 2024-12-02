#pragma once
#include "Scene.h"
#include <Vector2.h>
#include <Texture.h>
namespace E2
{
    class UIElement;
}

class WorldMap;
class WorldScene : public Scene
{
private:
    WorldMap* m_pWorldMap = nullptr;

    E2::UIElement* m_selectionRect = nullptr;

    E2::Texture m_worldMapTexture;
    bool m_hasEnded = false;
public:
    WorldScene(WorldMap* pMap);
    ~WorldScene();
    virtual bool Init() override;
    virtual void Update(float deltaTime) override;
    virtual SceneId GetId() override { return SceneId::WorldScene; }
    virtual void End() override;
    //virtual void Reset() override;

    void InitUI();
    void SetWorldMap(WorldMap* pWorldMap) { m_pWorldMap = pWorldMap; }
    void InputCheck();

    E2::Vector2 GetSelectionStart();
    E2::Vector2 GetSelectionEnd();

    void CreateWorldTexture();

    void TestDraw();
    void DrawWorldMap();

    void GotoLocalScene();
};