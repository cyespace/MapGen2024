#include "MapGenerator.h"
#include "LocalScene.h"
#include "WorldMap.h"
#include "Camera.h"
#include "Player.h"
#include "Wiki.h"

#include <GlobalFunctions.h>

#include <iostream>
#include <assert.h>

constexpr float kCreatureCountPerGrassTile = 0.05f;

constexpr float kMinIdleTime = 1.f;
constexpr float kMaxIdleTime = 5.f;
constexpr float kWanderRadius = 6.f; //unit as in tiles
constexpr float kWanderSpeed = 2.f; //unit as in pixels

constexpr int kMiniMapTileSize = 16;
constexpr int kSeaLevel = 6;

constexpr const char* kWikiData = "Script/CritterList.lua";
constexpr const char* kNorthIcon = "Script/UI/LocalScene/Image_NorthIcon.lua";

LocalScene::LocalScene(WorldMap* pMap)
    :m_pWorldMap{ pMap }
{
}

LocalScene::~LocalScene()
{
    if (!m_hasEnded)
    {
        End();
    }
}

bool LocalScene::Init()
{
    if (m_pWorldMap)
    {
        GetEngine().RegisterListener(this, kMouseEvent);

        CreateTexture();
        SetupCamera();
        InitWiki();
        SpawnPlayer();

        std::cout << "Local Scene Init.\n";
        return true;
    }
    else
    {
        std::cout << "Error: No world map!\n";
        return false;
    }
}

void LocalScene::Update(float deltaTime)
{
    InputCheck();

    switch (m_display)
    {
    case Display::BiomeColor: DrawMiniMap(); break;
    case Display::Isometric: DrawIsometric(); break;
    }
}

void LocalScene::End()
{
    GetEngine().RemoveListener(this, kMouseEvent);

    m_pWorldMap = nullptr;

    delete m_pCamera;
    m_pCamera = nullptr;

    delete m_pWiki;
    m_pWiki = nullptr;

    delete m_pPlayer;
    m_pPlayer = nullptr;

    GetEngine().ClearUI();
    GetEngine().ShowUI(true);

    m_hasEnded = true;
    std::cout << "Local Scene Ends.\n";
}

void LocalScene::OnNotify(E2::Event evt)
{
    //right click
    if (evt.m_mouseEvent.button == 3 && m_display == Display::BiomeColor)
    {
        GetTileInfo(evt.m_mouseEvent.x, evt.m_mouseEvent.y);
    }
}

void LocalScene::InputCheck()
{
    if (GetEngine().IsKeyPressed(E2::Keyboard::Key::Escape))
    {
        MapGenerator::Get().ViewWorld();
    }
    if (GetEngine().IsKeyPressed(E2::Keyboard::Key::Num1))
    {
        m_display = Display::Isometric;
    }
    if (GetEngine().IsKeyPressed(E2::Keyboard::Key::Num2))
    {
        m_display = Display::BiomeColor;
    }

    if (GetEngine().IsKeyPressed(E2::Keyboard::Key::Left) || GetEngine().IsKeyDown(E2::Keyboard::Key::Left))
    {
        m_pCamera->Move(-1, 0);
    }
    if (GetEngine().IsKeyPressed(E2::Keyboard::Key::Right) || GetEngine().IsKeyDown(E2::Keyboard::Key::Right))
    {
        m_pCamera->Move(1, 0);
    }
    if (GetEngine().IsKeyPressed(E2::Keyboard::Key::Up) || GetEngine().IsKeyDown(E2::Keyboard::Key::Up))
    {
        m_pCamera->Move(0, -1);
    }
    if (GetEngine().IsKeyPressed(E2::Keyboard::Key::Down) || GetEngine().IsKeyDown(E2::Keyboard::Key::Down))
    {
        m_pCamera->Move(0 ,1);
    }

    if (GetEngine().IsKeyPressed(E2::Keyboard::Key::A))
    {
        TryMovePlayer(-1, 0);
    }
    if (GetEngine().IsKeyPressed(E2::Keyboard::Key::D))
    {
        TryMovePlayer(1, 0);
    }
    if (GetEngine().IsKeyPressed(E2::Keyboard::Key::W))
    {
        TryMovePlayer(0, -1);
    }
    if (GetEngine().IsKeyPressed(E2::Keyboard::Key::S))
    {
        TryMovePlayer(0, 1);
    }
}

//
void LocalScene::SetViewRange(E2::Rect rect)
{
    m_startIndex.x = rect.x;
    m_startIndex.y = rect.y;
    m_size.x = rect.w;
    m_size.y = rect.h;
}

#if 0
void LocalScene::SpawnCreatures()
{
    std::unordered_map<CreatureType, E2::Texture> creatureTextureMap;
    creatureTextureMap[CreatureType::Bug] = GetEngine().CreateTexture("Assets/Creature/Bug.png");
    creatureTextureMap[CreatureType::Snow] = GetEngine().CreateTexture("Assets/Creature/Snow.png");
    creatureTextureMap[CreatureType::Green] = GetEngine().CreateTexture("Assets/Creature/Green.png");
    creatureTextureMap[CreatureType::Worm] = GetEngine().CreateTexture("Assets/Creature/Worm.png");
    creatureTextureMap[CreatureType::Pink] = GetEngine().CreateTexture("Assets/Creature/Pink.png");
    creatureTextureMap[CreatureType::Dry] = GetEngine().CreateTexture("Assets/Creature/Dry.png");

    auto goodTiles = m_pLocalMap->GetSpawnAbleTiles();

    if (goodTiles.empty())
    {
        return;
    }

    //calculate how many creature there shall be
    int creatureCount = (int)((float)goodTiles.size() * kCreatureCountPerGrassTile);

    for (int i = 0; i < creatureCount; ++i)
    {
        //find a random spot
        int tileId = -1;
        while (tileId == -1)
        {
            auto choice = E2::Random(0, goodTiles.size() - 1);
            tileId = goodTiles[choice];
            goodTiles[choice] = -1;
        }
        auto* pTile = m_pLocalMap->GetTile(tileId);

        auto creatureType = CreatureType::Unknown;
        switch (pTile->biomeId)
        {
        case 7:  creatureType = CreatureType::Green; break;
        case 8:  creatureType = CreatureType::Bug; break;
        case 9:  creatureType = CreatureType::Green; break;
        case 10: creatureType = CreatureType::Pink; break;
        case 11: creatureType = CreatureType::Worm; break;
        case 12: creatureType = CreatureType::Worm; break;
        case 13: creatureType = CreatureType::Snow; break;
        case 14: creatureType = CreatureType::Pink; break;
        case 15: creatureType = CreatureType::Dry; break;
        case 22: creatureType = CreatureType::Snow; break;
        default: assert(false && "No creature type."); break;
        }
        //new object
        auto* pNewObject = new E2::GameObject;
        auto* pImageComp = new E2::ImageComponent(pNewObject, creatureTextureMap[creatureType]);
        pNewObject->AddComponent(pImageComp);

        auto rect = pTile->rect;
        pNewObject->GetTransform()->SetPosition((float)rect.x, (float)rect.y);
        pNewObject->GetTransform()->SetDimension((float)rect.w, (float)rect.h);
        GetEngine().AddGameObject(pNewObject);

        //Add states
        auto* pIdleState = new IdleState(pNewObject,kMinIdleTime,kMaxIdleTime);
        auto* pWanderState = new WanderState(pNewObject,kWanderRadius,kWanderSpeed);
        auto* pTransitionToWander = new TransitionToWander(pNewObject);
        auto* pTransitionToIdle = new TransitionToIdle(pNewObject);
        auto* pStateMachine = new E2::StateComponent();
        pStateMachine->AddState(pIdleState, pTransitionToWander, pWanderState);
        pStateMachine->AddState(pWanderState, pTransitionToIdle, pIdleState);

        pStateMachine->SetState(pIdleState);
        pNewObject->AddComponent(pStateMachine);
    }
}
#endif

void LocalScene::SetupCamera(int magnitude)
{
    m_pCamera = new Camera;

    m_pCamera->SetLimit(m_pWorldMap->GetColumnCount(),m_pWorldMap->GetRowCount());
    m_pCamera->SetSize(m_size.x * magnitude, m_size.y * magnitude);
    //position has to be set at the end
    m_pCamera->SetPosition(m_startIndex.x * magnitude, m_startIndex.y * magnitude);
    m_display = Display::Isometric;

    GetEngine().Lua_LoadUIElement(kNorthIcon);
}

void LocalScene::CreateTexture()
{
    m_blockTextures[BlockType::Dirt].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_dirt_01.png"));
    m_blockTextures[BlockType::Dirt].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_dirt_02.png"));
    m_blockTextures[BlockType::Dirt].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_dirt_03.png"));
    m_blockTextures[BlockType::Dirt].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_dirt_04.png"));
    m_blockTextures[BlockType::Dirt].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_dirt_05.png"));
    m_blockTextures[BlockType::Dirt].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_dirt_06.png"));
    m_blockTextures[BlockType::Dirt].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_dirt_07.png"));
    m_blockTextures[BlockType::Dirt].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_dirt_08.png"));
    m_blockTextures[BlockType::Dirt].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_dirt_09.png"));

    m_blockTextures[BlockType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_ice_01.png"));
    m_blockTextures[BlockType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_ice_02.png"));
    m_blockTextures[BlockType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_ice_03.png"));
    m_blockTextures[BlockType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_ice_04.png"));
    m_blockTextures[BlockType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_ice_05.png"));
    m_blockTextures[BlockType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_ice_06.png"));
    m_blockTextures[BlockType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_ice_07.png"));
    m_blockTextures[BlockType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_ice_08.png"));
    m_blockTextures[BlockType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_ice_09.png"));

    m_blockTextures[BlockType::Rock].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_rock_01.png"));
    m_blockTextures[BlockType::Rock].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_rock_02.png"));
    m_blockTextures[BlockType::Rock].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_rock_03.png"));
    m_blockTextures[BlockType::Rock].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_rock_04.png"));
    m_blockTextures[BlockType::Rock].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_rock_05.png"));
    m_blockTextures[BlockType::Rock].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_rock_06.png"));
    m_blockTextures[BlockType::Rock].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_rock_07.png"));
    m_blockTextures[BlockType::Rock].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_rock_08.png"));
    m_blockTextures[BlockType::Rock].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_rock_09.png"));

    m_blockTextures[BlockType::Sand].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_sand_01.png"));
    m_blockTextures[BlockType::Sand].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_sand_02.png"));
    m_blockTextures[BlockType::Sand].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_sand_03.png"));
    m_blockTextures[BlockType::Sand].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_sand_04.png"));
    m_blockTextures[BlockType::Sand].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_sand_05.png"));
    m_blockTextures[BlockType::Sand].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_sand_06.png"));
    m_blockTextures[BlockType::Sand].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_sand_07.png"));
    m_blockTextures[BlockType::Sand].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_sand_08.png"));
    m_blockTextures[BlockType::Sand].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_sand_09.png"));

    m_blockTextures[BlockType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_water_01.png"));
    m_blockTextures[BlockType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_water_02.png"));
    m_blockTextures[BlockType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_water_03.png"));
    m_blockTextures[BlockType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_water_04.png"));
    m_blockTextures[BlockType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_water_05.png"));
    m_blockTextures[BlockType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_water_06.png"));
    m_blockTextures[BlockType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_water_07.png"));
    m_blockTextures[BlockType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_water_08.png"));
    m_blockTextures[BlockType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Block/b_water_09.png"));

    m_surfaceTextures[SurfaceType::Debug].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_debug_00.png"));
    m_surfaceTextures[SurfaceType::Debug].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_debug_01.png"));
    m_surfaceTextures[SurfaceType::Debug].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_debug_02.png"));
    m_surfaceTextures[SurfaceType::Debug].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_debug_03.png"));
    m_surfaceTextures[SurfaceType::Debug].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_debug_04.png"));
    m_surfaceTextures[SurfaceType::Debug].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_debug_05.png"));
    m_surfaceTextures[SurfaceType::Debug].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_debug_06.png"));
    m_surfaceTextures[SurfaceType::Debug].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_debug_07.png"));
    m_surfaceTextures[SurfaceType::Debug].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_debug_08.png"));
    m_surfaceTextures[SurfaceType::Debug].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_debug_09.png"));

    m_surfaceTextures[SurfaceType::Grass].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_grass_01.png"));
    m_surfaceTextures[SurfaceType::Grass].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_grass_02.png"));
    m_surfaceTextures[SurfaceType::Grass].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_grass_03.png"));
    m_surfaceTextures[SurfaceType::Grass].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_grass_04.png"));
    m_surfaceTextures[SurfaceType::Grass].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_grass_05.png"));
    m_surfaceTextures[SurfaceType::Grass].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_grass_06.png"));
    m_surfaceTextures[SurfaceType::Grass].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_grass_07.png"));
    m_surfaceTextures[SurfaceType::Grass].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_grass_08.png"));
    m_surfaceTextures[SurfaceType::Grass].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_grass_09.png"));

    m_surfaceTextures[SurfaceType::Snow].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_snow_01.png"));
    m_surfaceTextures[SurfaceType::Snow].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_snow_02.png"));
    m_surfaceTextures[SurfaceType::Snow].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_snow_03.png"));
    m_surfaceTextures[SurfaceType::Snow].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_snow_04.png"));
    m_surfaceTextures[SurfaceType::Snow].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_snow_05.png"));
    m_surfaceTextures[SurfaceType::Snow].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_snow_06.png"));
    m_surfaceTextures[SurfaceType::Snow].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_snow_07.png"));
    m_surfaceTextures[SurfaceType::Snow].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_snow_08.png"));
    m_surfaceTextures[SurfaceType::Snow].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_snow_09.png"));

    m_surfaceTextures[SurfaceType::Forest].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_forest_01.png"));
    m_surfaceTextures[SurfaceType::Forest].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_forest_02.png"));
    m_surfaceTextures[SurfaceType::Forest].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_forest_03.png"));
    m_surfaceTextures[SurfaceType::Forest].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_forest_04.png"));
    m_surfaceTextures[SurfaceType::Forest].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_forest_05.png"));
    m_surfaceTextures[SurfaceType::Forest].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_forest_06.png"));
    m_surfaceTextures[SurfaceType::Forest].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_forest_07.png"));
    m_surfaceTextures[SurfaceType::Forest].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_forest_08.png"));
    m_surfaceTextures[SurfaceType::Forest].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_forest_09.png"));

    m_surfaceTextures[SurfaceType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_water_01.png"));
    m_surfaceTextures[SurfaceType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_water_02.png"));
    m_surfaceTextures[SurfaceType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_water_03.png"));
    m_surfaceTextures[SurfaceType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_water_04.png"));
    m_surfaceTextures[SurfaceType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_water_05.png"));
    m_surfaceTextures[SurfaceType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_water_06.png"));
    m_surfaceTextures[SurfaceType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_water_07.png"));
    m_surfaceTextures[SurfaceType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_water_08.png"));
    m_surfaceTextures[SurfaceType::Water].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_water_09.png"));

    m_surfaceTextures[SurfaceType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_ice_01.png"));
    m_surfaceTextures[SurfaceType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_ice_02.png"));
    m_surfaceTextures[SurfaceType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_ice_03.png"));
    m_surfaceTextures[SurfaceType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_ice_04.png"));
    m_surfaceTextures[SurfaceType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_ice_05.png"));
    m_surfaceTextures[SurfaceType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_ice_06.png"));
    m_surfaceTextures[SurfaceType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_ice_07.png"));
    m_surfaceTextures[SurfaceType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_ice_08.png"));
    m_surfaceTextures[SurfaceType::Ice].emplace_back(GetEngine().CreateTexture("Assets/Isometric/Surface/s_ice_09.png"));

    m_treeTextures[TreeType::Cold] = GetEngine().CreateTexture("Assets/Isometric/Element/e_northTree.png");
    m_treeTextures[TreeType::Dead] = GetEngine().CreateTexture("Assets/Isometric/Element/e_cactusTree.png");
    m_treeTextures[TreeType::Green] = GetEngine().CreateTexture("Assets/Isometric/Element/e_tree.png");
    m_treeTextures[TreeType::HalfSnow] = GetEngine().CreateTexture("Assets/Isometric/Element/e_firTree.png");
    m_treeTextures[TreeType::Rain] = GetEngine().CreateTexture("Assets/Isometric/Element/e_tropicalTree.png");
    m_treeTextures[TreeType::Yellow] = GetEngine().CreateTexture("Assets/Isometric/Element/e_yellowTree.png");

    m_grassTextures[GrassType::Cold] = GetEngine().CreateTexture("Assets/Isometric/Element/e_tundraGrass.png");
    m_grassTextures[GrassType::Green] = GetEngine().CreateTexture("Assets/Isometric/Element/e_grass.png");
    m_grassTextures[GrassType::HalfSnow] = GetEngine().CreateTexture("Assets/Isometric/Element/e_snowGrass.png");
    m_grassTextures[GrassType::Rain] = GetEngine().CreateTexture("Assets/Isometric/Element/e_forestGrass.png");
    m_grassTextures[GrassType::Tall] = GetEngine().CreateTexture("Assets/Isometric/Element/e_tallGrass.png");
    m_grassTextures[GrassType::Yellow] = GetEngine().CreateTexture("Assets/Isometric/Element/e_dryGrass.png");

    m_houseTextures[HouseType::SouthHouse] = GetEngine().CreateTexture("Assets/Isometric/Element/e_house_01.png");
    m_houseTextures[HouseType::EastHouse] = GetEngine().CreateTexture("Assets/Isometric/Element/e_house_02.png");

    m_townRoadTexture = GetEngine().CreateTexture("Assets/Isometric/Surface/s_townRoad.png");
    m_worldRoadTexture = GetEngine().CreateTexture("Assets/Isometric/Surface/s_worldRoad.png");
    m_playerTexture = GetEngine().CreateTexture("Assets/Isometric/Critter/c_player.png"); 

    m_critterTextures.emplace_back();
    m_critterTextures.emplace_back(GetEngine().CreateTexture("Assets/Isometric/Critter/c_01.png"));
    m_critterTextures.emplace_back(GetEngine().CreateTexture("Assets/Isometric/Critter/c_02.png"));
    m_critterTextures.emplace_back(GetEngine().CreateTexture("Assets/Isometric/Critter/c_03.png"));
}

void LocalScene::GetTileInfo(int coordX, int coordY)
{
    int tileLength = kMiniMapTileSize;

    int x0 = m_pCamera->GetWindow().x;
    int y0 = m_pCamera->GetWindow().y;

    int x = coordX / tileLength + x0;
    int y = coordY / tileLength + y0;

    Tile* pTile = m_pWorldMap->GetTile(x,y);
    if (!pTile)
    {
        return;
    }

    std::cout << "*************** Clicked Tile: ***************\n";
    std::cout << "Index = " << pTile->m_index << "\n";
    std::cout << "Global X = " << pTile->m_x << ",Global Y = " << pTile->m_y << "\n";
    std::cout << "Local X = " << coordX / tileLength << ",Local Y = " << coordY / tileLength << "\n";
    std::cout << "HeightNoise = " << pTile->m_heightNoise << "\n";
    std::cout << "TemperatureNoise = " << pTile->m_temperatureNoise << "\n";
    std::cout << "Terrain Height = " << pTile->m_height << "\n";
    std::cout << "Biome ID = " << pTile->m_biomeId << "\n";
    std::cout << "Block Position ID= " << pTile->m_blockId << "\n";
    std::cout << "Surface Position ID= " << pTile->m_surfaceId << "\n";
    std::cout << "Town Road Position ID= " << pTile->m_townRoadId << "\n";
    std::cout << "World Road Position ID= " << pTile->m_worldRoadId << "\n";
    std::cout << "*********************************************\n";
}

void LocalScene::DrawMiniMap()
{
    int tileSize = kMiniMapTileSize;

    auto drawWindow = m_pCamera->GetWindow();

    for (int y = 0; y < drawWindow.h; ++y)
    {
        for (int x = 0; x < drawWindow.w; ++x)
        {
            auto* pTile = m_pWorldMap->GetTile(x + drawWindow.x, y + drawWindow.y);
            assert(pTile);
            E2::Rect rect{ x * tileSize,y * tileSize, tileSize,tileSize };

            if(pTile->m_isTownRoad)
            {
                DrawRect(rect, { 255,0,255 });
            }
            else if (pTile->m_isWorldRoad)
            {
                DrawRect(rect, { 255,0,255 });
            }
            else if (pTile->m_stuff == Tile::Stuff::SouthHouse || pTile->m_stuff == Tile::Stuff::EastHouse)
            {
                if (pTile->m_index == pTile->m_isOccupiedBy)
                {
                    DrawRect(rect, { 255,0,0 });
                }
                if (pTile->m_isDoor)
                {
                    DrawRect(rect, { 0,0,255 });
                }
            }
            else
            {
                DrawRect(rect, pTile->m_biomeColor);
            }
        }
    }
}

void LocalScene::DrawIsometric()
{
    auto drawWindow = m_pCamera->GetWindow();

    static int screenCenterX = GetEngine().GetWindowSize().x / 2;
    // n(L/4), k = length of a isometric tile = 32, n = tile count = number of tiles display horizontally 
    // total adjustment = half of camera window height - isometric offset + height offset
    static int screenCenterY = (GetEngine().GetWindowSize().y / 2) - (8 * drawWindow.w) + (m_pWorldMap->GetMaxHeight() * 8);

    E2::Rect srcRect;
    E2::Rect destRect;

    for (int y = 0; y < drawWindow.h; ++y)
    {
        for (int x = 0; x < drawWindow.w; ++x)
        {
            auto* pTile = m_pWorldMap->GetTile(x + drawWindow.x, y + drawWindow.y);
            assert(pTile);
            int x1 = (x * 16) - (y * 16);
            int y1 = (x * 8) + (y * 8);
            BlockType blockType = TypePicker::GetBlockTypeFromBiome(pTile->m_biomeId);
            SurfaceType surfaceType = TypePicker::GetSurfaceTypeFromBiome(pTile->m_biomeId);
            TreeType treeType = TypePicker::GetTreeTypeFromBiome(pTile->m_biomeId);
            GrassType grassType = TypePicker::GetGrassTypeFromBiome(pTile->m_biomeId);

            auto* pEastTile = m_pWorldMap->GetNeighborTile(pTile->m_index, WorldMap::Direction::E);
            auto* pSouthTile = m_pWorldMap->GetNeighborTile(pTile->m_index, WorldMap::Direction::S);

            for (int currentHeight = 0; currentHeight < pTile->m_height + 1; ++currentHeight) //the height start from 0, adding 1 to not skip tiles with 0 height
            {
                bool isTop = (currentHeight == pTile->m_height);

                //check east and south block, if it's blocked then skip
                if (!isTop)
                {
                    if (pEastTile && pSouthTile)
                    {
                        if (currentHeight <= pEastTile->m_height && currentHeight <= pSouthTile->m_height)
                        {
                            continue;
                        }
                    }
                }

                //draw block
                int yOffset = currentHeight * 16;
                destRect = { x1 - 16 + screenCenterX ,y1 - yOffset + screenCenterY ,32,32 };
                DrawTexture((m_blockTextures[blockType])[pTile->m_blockId], nullptr, &destRect);

                //if it is the top layer, draw the surface, if avaliable 
                if (isTop && currentHeight >= kSeaLevel)
                {
                    //surface
                    if (surfaceType != SurfaceType::None)
                    {
                        DrawTexture((m_surfaceTextures[surfaceType])[pTile->m_surfaceId], nullptr, &destRect);
                    }

                    if (pTile->m_isWorldRoad)
                    {
                        assert(pTile->m_worldRoadId != BAD && "LocalScene::DrawIsometric(): world road misplaced\n");
                        srcRect = { 32 * pTile->m_worldRoadId,0,32,32 };
                        DrawTexture(m_worldRoadTexture, &srcRect, &destRect);
                    }

                    if (pTile->m_isTownRoad)
                    {
                        assert(pTile->m_townRoadId != BAD && "LocalScene::DrawIsometric(): town road misplaced\n");
                        srcRect = { 32 * pTile->m_townRoadId,0,32,32 };
                        DrawTexture(m_townRoadTexture, &srcRect, &destRect);
                    }

                    //house
                    if (pTile->m_stuff == Tile::Stuff::SouthHouse)
                    {
                        E2::Rect source = { pTile->m_houseTextureId * 32, 0, 32, 64 };
                        auto houseRect = destRect;
                        houseRect.y -= 32;
                        houseRect.w = 32;
                        houseRect.h = 64;
                        DrawTexture(m_houseTextures[HouseType::SouthHouse], &source, &houseRect);
                    }
                    else if (pTile->m_stuff == Tile::Stuff::EastHouse)
                    {
                        E2::Rect source = { pTile->m_houseTextureId * 32, 0, 32, 64 };
                        auto houseRect = destRect;
                        houseRect.y -= 32;
                        houseRect.w = 32;
                        houseRect.h = 64;
                        DrawTexture(m_houseTextures[HouseType::EastHouse], &source, &houseRect);
                    }

                    //tree
                    if (treeType != TreeType::None && pTile->m_treeNoise > 0)
                    {
                        auto treeRect = destRect;
                        treeRect.y -= 32;
                        treeRect.h = 48;
                        DrawTexture(m_treeTextures[treeType], nullptr, &treeRect);
                    }
                    //grass
                    if (grassType != GrassType::None && pTile->m_treeNoise < 0 && pTile->m_grassNoise > 0)
                    {
                        auto grassRect = destRect;
                        grassRect.y -= 16;
                        DrawTexture(m_grassTextures[grassType], nullptr, &grassRect);

                        //critters
                        if (pTile->m_critterId != BAD)
                        {
                            DrawTexture(m_critterTextures[pTile->m_critterId], nullptr, &grassRect);
                        }
                    }

                    if (m_pPlayer->IsAlive())
                    {
                        if (m_pPlayer->GetPosition().x == pTile->m_x && m_pPlayer->GetPosition().y == pTile->m_y)
                        {
                            auto playerRect = destRect;
                            playerRect.y -= 16;
                            DrawTexture(m_playerTexture, nullptr, &playerRect);
                        }
                    }
                }
                //TODO: Sealevel problem
                else if (isTop && currentHeight <= kSeaLevel && surfaceType != SurfaceType::None)
                {
                    auto waterSurface = destRect;
                    int offset = kSeaLevel - pTile->m_height;
                    waterSurface.y -= (offset * 16);
                    DrawTexture((m_surfaceTextures[surfaceType])[pTile->m_surfaceId], nullptr, &waterSurface);
                }
            }
        }
    }
}

void LocalScene::SpawnPlayer()
{
    if (m_pPlayer && m_pPlayer->IsAlive())
        return;

    m_pPlayer = new Player;

    auto tileIsGoodToSpawn = [](Tile* pTile)->bool
    {
        if (pTile->m_treeNoise < 1)
        {
            if (pTile->m_height > 7 && pTile->m_height < 13)
            {
                return true;
            }
        }
        return false;
    };

    auto window = m_pCamera->GetWindow();
    //find a good tile near the center
    int x = window.x + window.w / 2;
    int y = window.y + window.h / 2;

    auto pool = m_pWorldMap->GetTilesInRadius(m_pWorldMap->GetTileIndex(x, y),3,true);

    for (int id : pool)
    {
        auto* pTile = m_pWorldMap->GetTile(id);

        if (tileIsGoodToSpawn(pTile))
        {
            x = pTile->m_x;
            y = pTile->m_y;
            m_pPlayer->SetPosition(x, y);
            m_pPlayer->SetAlive(true);

            //center the camera on the player
            m_pCamera->SetCenter(x, y);
            break;
        }
    }
}

void LocalScene::TryMovePlayer(int deltaX, int deltaY)
{
    if (!m_pPlayer || !m_pPlayer->IsAlive())
        return;
    int testX = m_pPlayer->GetPosition().x + deltaX;
    int testY = m_pPlayer->GetPosition().y + deltaY;

    auto* pTile = m_pWorldMap->GetTile(testX, testY);
    if(pTile)
    {
        if (pTile->m_height >= 6 && pTile->m_height < 13)
        {
            if (pTile->m_stuff == Tile::Stuff::Nothing)
            {
                if (pTile->m_treeNoise < 1)
                {
                    if (pTile->m_critterId == BAD)
                    {
                        m_pPlayer->SetPosition(testX, testY);
                        m_pCamera->SetCenter(testX, testY);
                        if(m_pWiki->IsOpen())
                            m_pWiki->Close();
                    }
                    else
                    {
                        //collect
                        m_pPlayer->CatchCritter(pTile->m_critterId);
                        m_pWiki->DiscoverCritter(pTile->m_critterId);
                        m_pWiki->Open();
                        pTile->m_critterId = BAD;
                        m_pPlayer->SetPosition(testX, testY);
                        m_pCamera->SetCenter(testX, testY);
                    }
                }
            }
        }
    }
}

void LocalScene::InitWiki()
{
    m_pWiki = new Wiki;
    m_pWiki->Init();
}

//E2::Vector2f LocalScene::GetRandomNearPositionInNav(E2::Vector2f origin, float radius)
//{
//    return m_pLocalMap->GetRandomNearPositionInNav(origin,radius);
//}
//
//std::vector<E2::Vector2f> LocalScene::BuildPath(E2::Vector2f from, E2::Vector2f to)
//{
//    return m_pLocalMap->BuildPath(from, to);
//}
