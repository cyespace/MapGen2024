#include "MapGenerator.h"
#include "WorldMap.h"
#include "MenuScene.h"
#include "WorldScene.h"
#include "LocalScene.h"
#include "MacroDef.h"
#include "TerraForm.h"
#include "Critters/CritterFactory.h"
#include "Region.h"
#include "Wiki.h"

#include <GlobalFunctions.h>
#include <HashString.h>

#include <GameObject.h>
#include <TransformComponent.h>

constexpr const char* kConfig = "Script/Config.lua";
constexpr const char* kGameAssets = "Script/GameAssetIndex.lua";

MapGenerator& MapGenerator::Get()
{
    static MapGenerator instance;
    return instance;
}

MapGenerator::MapGenerator()
{
    //

}

MapGenerator::~MapGenerator()
{
    ShutDown();
}

bool MapGenerator::Init()
{
    GetEngine().LoadFilesFromScript(kGameAssets);

    m_pNextScene = new MenuScene;
    return true;
}

void MapGenerator::Update(float deltaTime)
{
    if (m_pCurrentScene)
    {
        m_pCurrentScene->Update(deltaTime);
    }
    ChangeScene();
}

void MapGenerator::ShutDown()
{
    delete m_pCurrentScene;
    m_pCurrentScene = nullptr;
    delete m_pNextScene;
    m_pNextScene = nullptr;

    delete m_pMap;
    m_pMap = nullptr;
}

const char* MapGenerator::Config()
{
    return kConfig;
}

void MapGenerator::SetMapSeed(std::string& seed)
{
    m_mapSeed = E2::HashString(seed);
}

void MapGenerator::SetMapSeed(size_t seed)
{
    m_mapSeed = seed;
    E2::Rand::SetSeed(seed);
}

void MapGenerator::ChangeScene()
{
    if (m_pNextScene)
    {
        if (m_pCurrentScene)
        {
            m_pCurrentScene->End();
            delete m_pCurrentScene;
        }
        bool bSceneInit = m_pNextScene->Init();
        assert(bSceneInit && "Scene Init failed.");
        if(bSceneInit)
        {
            m_pCurrentScene = m_pNextScene;
            m_pNextScene = nullptr;
        }
    }
}

void MapGenerator::ViewLocal(E2::Rect rect)
{
    auto* pLocal = new LocalScene(m_pMap);
    pLocal->SetViewRange(rect);
    m_pNextScene = pLocal;
}

void MapGenerator::ViewWorld()
{
    m_pNextScene = new WorldScene(m_pMap);;
}

void MapGenerator::Restart()
{
    m_pNextScene = new MenuScene;
}

void MapGenerator::GenerateWorld()
{
    delete m_pMap;
    m_pMap = new WorldMap();
    m_pMap->Init(kMapSizeX, kMapSizeY);
    {
        using namespace TerraForm;
        //height noise
        BasicPerlinNoise(NoiseType::Height, m_pMap, kDefaultOctaves, kDefaultInputRange, kDefaultPersistence);
        StretchNoise(NoiseType::Height, m_pMap);
        //Regional height noise change
        auto* pRegion = new Region();
        SplitRegion(pRegion, m_pMap, kMaxWorldSplits, kMinSplitSize);
        AdjustRegionalHeightNoise(pRegion, m_pMap);
        SmoothNoiseCrossRegion(pRegion, m_pMap, kSmoothDistanceFromSplit);
        delete pRegion;

        StretchNoise(NoiseType::Height, m_pMap);
        FormulateHeight(m_pMap, kMaxHeight);

        //temperature
        BasicPerlinNoise(NoiseType::Temperature, m_pMap, 1, kTemperatureInputRange, 1);   //do only 1 iteration
        StretchNoise(NoiseType::Temperature, m_pMap);
        int halfMapLength = m_pMap->GetRowCount() / 2;
        AdjustTemperatureNoise(m_pMap, halfMapLength);

        //Biome
        PickBiome(m_pMap, kBiomeData);

        m_pMap->Expand(kMapExpandMagnitude);  //Expansion must happen

        //Textures
        FindBlockId(m_pMap);
        FindSurfaceId(m_pMap);

        //Tree and Grass
        BasicPerlinNoise(NoiseType::Tree, m_pMap, 1, kTreeDensity, 1);   //do only 1 iteration
        BasicPerlinNoise(NoiseType::Grass, m_pMap, 1, kTreeDensity, 1);   //do only 1 iteration
        CompressNoise(NoiseType::Tree, m_pMap, kTreeThreashold);
        CompressNoise(NoiseType::Grass, m_pMap, kTreeThreashold);
        SortTreeAndGrass(m_pMap);
         
        //TODO : houses
        auto townCenters = FindTownCenter(m_pMap, kTownDensity, kSeaHeight, 12, 0.2f, 0.8f);
        m_pMap->SetTownCenters(townCenters);

        auto houseDoors = BuildTown(m_pMap,kTownSize, 2.75f);
        m_pMap->SetHouseDoors(houseDoors);

        BuildRoadsBetweenHouses(m_pMap, 10);
        BuildRoadsBetweenTowns(m_pMap, kTownDensity);
        FindTownRoadId(m_pMap);
        FindWorldRoadId(m_pMap);

    }
    CritterFactory::SpawnCritters(m_pMap);
}