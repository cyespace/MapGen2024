#pragma once
#include "Scene.h"

#include <IGame.h>
#include <Vector2.h>
#include <Rect.h>
#include <vector>


namespace E2
{
    struct Color;
    class UIElement;
    class GameObject;
}

class WorldMap;
class Scene;
class Wiki;
class MapGenerator : public E2::IGame
{
private:
    size_t m_mapSeed = 0;

    WorldMap* m_pMap = nullptr; //TODO: unique

    //Scene
    Scene* m_pCurrentScene = nullptr;
    Scene* m_pNextScene = nullptr;

public:
    static MapGenerator& Get();
    virtual ~MapGenerator();
    virtual bool Init() final;
    virtual void Update(float deltaTime) final;
    virtual void ShutDown() final;
    virtual const char* Config() final;

    //seed
    void SetMapSeed(std::string& seed);
    void SetMapSeed(size_t seed);
    size_t GetMapSeed()const { return m_mapSeed; }

    //scene
    void ChangeScene();
    void ViewLocal(E2::Rect rect);
    void ViewWorld();

    //World Generation
    void Restart();
    void GenerateWorld();

    WorldMap* GetWorldMap() { return m_pMap; }

private:
    MapGenerator();
};