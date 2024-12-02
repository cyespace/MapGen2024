#pragma once
#include "Scene.h"
#include "LocalMapElementDef.h"


#include <Vector2.h>
#include <Texture.h>
#include <EventListener.h>

#include <unordered_map>
class WorldMap;
class Camera;
class Player;
class Wiki;
class LocalScene : public Scene, public E2::EventListener
{
    enum class Display
    {
        BiomeColor,
        Isometric,
        Wiki,
    };

    enum class CreatureType
    {
        Unknown,
        Green,
        Snow,
        Worm,
        Pink,
        Bug,
        Dry,
    };
    
    using BlockBox = std::unordered_map<BlockType, std::vector<E2::Texture>>;
    using SurfaceBox = std::unordered_map<SurfaceType, std::vector<E2::Texture>>;
    using TreeBox = std::unordered_map<TreeType, E2::Texture>;
    using GrassBox = std::unordered_map<GrassType, E2::Texture>;
    using HouseBox = std::unordered_map<HouseType, E2::Texture>;
    using CritterBox = std::vector<E2::Texture>;

private:
    bool m_hasEnded = false;
    WorldMap* m_pWorldMap = nullptr;
    Camera* m_pCamera = nullptr;

    Display m_display = Display::Isometric;

    //TODO: remove these 2 vector
    E2::Vector2 m_startIndex;
    E2::Vector2 m_size;
    
    Player* m_pPlayer = nullptr;
    Wiki* m_pWiki = nullptr;

    E2::UIElement* pWikiImage = nullptr;
    E2::UIElement* pWikiText = nullptr;

    //Textures 
    BlockBox m_blockTextures;
    SurfaceBox m_surfaceTextures;
    TreeBox m_treeTextures;
    GrassBox m_grassTextures;
    HouseBox m_houseTextures;
    CritterBox m_critterTextures;
    E2::Texture m_townRoadTexture;
    E2::Texture m_worldRoadTexture;
    E2::Texture m_playerTexture;
public:
    LocalScene(WorldMap* pMap);
    ~LocalScene();
    virtual bool Init() override;
    virtual void Update(float deltaTime) override;
    virtual SceneId GetId() override { return SceneId::LocalScene; }
    virtual void End() override;
    virtual void OnNotify(E2::Event evt) override;

    void InputCheck();

    void SetWorldMap(WorldMap* pWorld) { m_pWorldMap = pWorld; }
    void SetViewRange(E2::Rect rect);

    void SpawnCreatures();

    //test
    void SetupCamera(int magnitude = 3);
    void CreateTexture();
    void GetTileInfo(int coordX, int coordY);
    void DrawMiniMap();
    void DrawIsometric();

    void SpawnPlayer();
    void TryMovePlayer(int deltaX, int deltaY);

    void InitWiki();
    //nav
    //E2::Vector2f GetRandomNearPositionInNav(E2::Vector2f origin, float radius);
    //std::vector<E2::Vector2f> BuildPath(E2::Vector2f from, E2::Vector2f to);
};