#include "LocalMap.h"
#include "HeightBiomes.h"
#include "Grid.h"
#include "Region.h"
#include "MapGenerator.h"
#include "LocalScene.h"

#include <Random.h>
#include <FreeFunctions.h>
#include <PerlinNoise.h>

#include <Iostream>
#include <map>
#include <queue>
#include <algorithm>

constexpr float kColorValue = 0.7f;
constexpr int kCitySplitTimes = 10;
constexpr int kMinSplitTileCount = 50;
constexpr int kMinSplitSize = 8;
constexpr int kBuildingSizeInverse = 3;

LocalMap::LocalMap()
    : m_pGrid{nullptr}
{
}

LocalMap::~LocalMap()
{
    delete m_pGrid;
    for (auto* pRegion : m_rootRegions)
    {
        delete pRegion;
    }
    for (auto* pObject : m_treesAndBuildings)
    {
        delete pObject;
    }
    for (auto* pObject : m_grass)
    {
        delete pObject;
    }
}

bool LocalMap::IsWalkAble(int tileId)
{
    return m_tileNodes[tileId].m_isWalkable;
}

void LocalMap::Reset(int maxX, int maxY, int tileSize)
{
    delete m_pGrid;
    for (auto* pRegion : m_rootRegions)
    {
        delete pRegion;
    }
    for (auto* pObject : m_treesAndBuildings)
    {
        delete pObject;
    }
    for (auto* pObject : m_grass)
    {
        delete pObject;
    }

    m_colors.clear();
    m_areas.clear();
    m_tileNodes.clear();
    m_tileTextures.clear();

    m_rootRegions.clear();
    m_cities.clear();
    m_pGrid = new Grid;
    m_pGrid->Init(maxX,maxY,tileSize);

    m_buildableTiles.clear();
}

void LocalMap::ReFreshTileNodesForSearching()
{
    for (auto& node : m_tileNodes)
    {
        node.m_isTested = false;
    }
}

void LocalMap::ReFreshPathNodesForPathing()
{
    for (auto& node : m_pathNodes)
    {
        node.m_gScore = std::numeric_limits<float>::max();
        node.m_fScore = node.m_gScore;
        node.m_cameFrom = -1;
    }
}

void LocalMap::CopyTileInfo(int index, Tile* pInTile)
{
    auto* pTile = m_pGrid->GetTile(index);
    pTile->type = pInTile->type;
    pTile->heightNoise = pInTile->heightNoise;
    pTile->rawTemperature = pInTile->rawTemperature;
    pTile->biomeId = pInTile->biomeId;
}

void LocalMap::CopyTileInfo(int x, int y, Tile* pInTile)
{
    auto* pTile = m_pGrid->GetTile(x,y);
    pTile->type = pInTile->type;
    pTile->heightNoise = pInTile->heightNoise;
    pTile->rawTemperature = pInTile->rawTemperature;
    pTile->biomeId = pInTile->biomeId;
}

void LocalMap::Terrace(int level)
{
    assert(level > 0);
    for (int i = 0; i < m_pGrid->Size(); ++i)
    {
        Tile* pTile = m_pGrid->GetTile(i);
        pTile->heightNoise = (float)(std::lroundf(pTile->heightNoise * (float)level)) / (float)level;
    }
}

std::vector<float> LocalMap::GetPerlinNoiseMap(float interval, float zoomInLevel)
{
    auto seed = E2::Random();
    std::vector<float> noiseMap;
    noiseMap.reserve(m_pGrid->Size());

    int gridWidth = m_pGrid->GetColumn(); //using width as long side 
    int gridHeight = m_pGrid->GetRow();

    float x0 = 0;
    float y0 = 0;

    for (int y = 0; y < gridHeight; ++y)
    {
        for (int x = 0; x < gridWidth; ++x)
        {
            if ((float)x > x0 + interval)
            {
                x0 += interval;
            }
            if (x == 0)
            {
                x0 = 0;
            }

            //range [-1, +1]
            float noise = PerlinNoise::Perlin((x0 / gridWidth) * zoomInLevel, (y0 / gridWidth) * zoomInLevel, seed);
            noiseMap.push_back(noise);

        }
        if ((float)y > y0 + interval)
        {
            y0 += interval;
        }
        if (y == 0)
        {
            y0 = 0;
        }
    }
    return noiseMap;
}

void LocalMap::SplitRegion(Region* pRoot)
{
    auto availableNodes = Region::FindSplittableRegions(pRoot);
    // TODO: make weighted random
    auto choice = E2::Random(0, availableNodes.size() - 1);
    availableNodes[choice]->Split(kMinSplitTileCount,kMinSplitSize);
}

void LocalMap::GenerateBuildableRegions()
{
    FindCities();
    for (auto& [start, end] : m_cities)
    {
        auto* pRegion = new Region();
        pRegion->m_region = { {start->x,start->y},{end->x,end->y} };
        m_rootRegions.push_back(pRegion);
        for (int i = 0; i < kCitySplitTimes; ++i)
        {
            SplitRegion(pRegion);
        }
    }
    SpawnBuildings();
}

// deprecated
#if(0)
void LocalMap::ConnectBuildings()
{
    std::map<int , std::vector<Region*>> openSet;
    std::vector<Region*> closeSet;

    auto sortRegion = [](Region* pRegion, std::map<int, std::vector<Region*>>* openSet)
    {
        (*openSet)[pRegion->m_depth].push_back(pRegion);
    };

    for (auto* pRoot : m_rootRegions)
    {
        Region::WalkTheTree(pRoot, sortRegion, &openSet);

        for (int i = (int)openSet.size(); i>=0; --i)
        {
            auto& regionVector = openSet[i];
            for (auto* pRegion : regionVector)
            {
                //check if this region is done already
                bool regionIsDone = false;
                for (auto* pClosedRegion : closeSet)
                {
                    if (pClosedRegion == pRegion)
                    {
                        regionIsDone = true;
                        break;
                    }
                }
                if (regionIsDone)
                {
                    continue;
                }

                //find sibling
                if (pRegion->m_pParent == nullptr)
                {
                    break;
                }
                Region* pSibling = nullptr;
                if (pRegion->m_pParent->m_pLeft == pRegion)
                {
                    pSibling = pRegion->m_pParent->m_pRight;
                }
                else
                {
                    pSibling = pRegion->m_pParent->m_pLeft;
                }
                assert(pSibling != pRegion);
                assert(pSibling->m_depth == pRegion->m_depth);

                //connect siblings

                int midX1 = 0;
                int midY1 = 0;
                int midX2 = 0;
                int midY2 = 0;

                if (!(pRegion->m_pLeft && pRegion->m_pRight))
                {
                    midX1 = (pRegion->m_innerRegion.first.x + pRegion->m_innerRegion.second.x) / 2;
                    midY1 = (pRegion->m_innerRegion.first.y + pRegion->m_innerRegion.second.y) / 2;
                }
                else
                {
                    std::vector<Tile*> roadTiles;
                    int startX = pRegion->m_innerRegion.first.x;
                    int startY = pRegion->m_innerRegion.first.y;
                    int endX = pRegion->m_innerRegion.second.x;
                    int endY = pRegion->m_innerRegion.second.y;
                    for (int y = startY; y <= endY; ++y)
                    {
                        for (int x = startX; x <= endX; ++x)
                        {
                            auto* pTile = m_pGrid->GetTile(x, y);
                            //if (pTile->type == Tile::Type::CityRoad)
                            {
                                roadTiles.push_back(pTile);
                            }
                        }
                    }
                    auto* pRoadMid = roadTiles[roadTiles.size()/2];
                    midX1 = pRoadMid->x;
                    midY1 = pRoadMid->y;
                }
                  

                if (!(pSibling->m_pLeft && pSibling->m_pRight))
                {
                    midX2 = (pSibling->m_innerRegion.first.x + pSibling->m_innerRegion.second.x) / 2;
                    midY2 = (pSibling->m_innerRegion.first.y + pSibling->m_innerRegion.second.y) / 2;
                }
                else
                {
                    std::vector<Tile*> roadTiles;
                    int startX = pSibling->m_innerRegion.first.x;
                    int startY = pSibling->m_innerRegion.first.y;
                    int endX = pSibling->m_innerRegion.second.x;
                    int endY = pSibling->m_innerRegion.second.y;
                    for (int y = startY; y <= endY; ++y)
                    {
                        for (int x = startX; x <= endX; ++x)
                        {
                            auto* pTile = m_pGrid->GetTile(x, y);
                            if (pTile->type == Tile::Type::CityRoad)
                            {
                                roadTiles.push_back(pTile);
                            }
                        }
                    }
                    auto* pRoadMid = roadTiles[roadTiles.size() / 2];
                    midX2 = pRoadMid->x;
                    midY2 = pRoadMid->y;
                }

                int startX = midX1 > midX2 ? midX2 : midX1;
                int endX = midX1 > midX2 ? midX1 : midX2;
                int startY = midY1 > midY2 ? midY2 : midY1;
                int endY = midY1 > midY2 ? midY1 : midY2;
                //TODO:Road Bug 
                //sometimes one tile off the road
                int usingX = 0;
                int usingY = 0;
                if (startY == midY1)
                {
                    usingX = midX2;
                }
                else
                {
                    usingX = midX1;
                }

                if (startX == midX1)
                {
                    usingY = midY2;
                }
                else
                {
                    usingY = midY1;
                }
                for (int y = startY; y <= endY; ++y)
                {
                    if (m_pGrid->GetTile(usingX, y)->type != Tile::Type::Building)
                    {
                        m_pGrid->GetTile(usingX, y)->type = Tile::Type::CityRoad;
                    }
                }
                for (int x = startX; x <= endX; ++x)
                {
                    if (m_pGrid->GetTile(x, usingY)->type != Tile::Type::Building)
                    {
                        m_pGrid->GetTile(x, usingY)->type = Tile::Type::CityRoad;
                    }
                }

                closeSet.push_back(pRegion);
                closeSet.push_back(pSibling);
            }
        }
        openSet.clear();
        closeSet.clear();
    }
}
#endif

void LocalMap::DrawHeight()
{
    for (int i = 0; i < m_pGrid->Size(); ++i)
    {
        Tile* pTile = m_pGrid->GetTile(i);
        E2::Color c = E2::Color_hsva(240.f - 120.f * (float)pTile->heightNoise, 1.f, kColorValue);
        DrawRect(pTile->rect, c);
    }
}

void LocalMap::DrawHeightTerrace(int level)
{
    for (int i = 0; i < m_pGrid->Size(); ++i)
    {
        Tile* pTile = m_pGrid->GetTile(i);
        auto newHeightNoise = (float)(std::lroundf(pTile->heightNoise * (float)level)) / (float)level;
        E2::Color c = E2::Color_hsva(240.f - 120.f * newHeightNoise,1.f, kColorValue);
        DrawRect(pTile->rect, c);
    }
}

void LocalMap::LoadArea(float level)
{
    //fomula height into whole numbers
    m_tileNodes.reserve(m_pGrid->Size());

    for (int i = 0; i < m_pGrid->Size(); ++i)
    {
        auto* pTile = m_pGrid->GetTile(i);
        auto height = (float)(std::lroundf(pTile->heightNoise * (float)level)) / (float)level;

        m_tileNodes.emplace_back(TileNode{ pTile->id, 0, height, true });
    }

    //build areas with same height
    for (int i = 0; i < m_tileNodes.size(); ++i)
    {
        if (!m_tileNodes[i].m_isTested) // untested node
        {
            BuildArea(i);
        }
    }

    m_colors.reserve(m_areas.size());
    for (int i=0; i< m_areas.size(); ++i)
    {
        m_colors.emplace_back(RandomColor());
    }

    //others
    GetTilePositionTest();
    LoadTerrainTexture();
    LoadTilesetTexture();

    ConnectAreas();
    GenerateBuildableRegions();
    BuildTown();
    BuildTownRoad();

    PopulateTrees();
    PopulateGrass();

    GetEngine().CleanRenderer();
    DrawArea();
    auto* pScene = dynamic_cast<LocalScene*>(MapGenerator::Get().GetScene(Scene::SceneId::LocalScene));
    pScene->SetLocalView(GetEngine().CombineCurrentView());
}

//Search the region, put all tiles with same height in same area
void LocalMap::BuildArea(int nodeId)
{
    auto testNeighbor = [](Tile* pNeighbor, std::vector<TileNode>& allNodes, int centerId, std::vector<int>& openSet)
    {
        if (pNeighbor) // quit if neighbor is out of screen
        {
            if (!allNodes[pNeighbor->id].m_isTested) // quit if neighbor is already tested
            {
                // if neighbor and this tile have same height, they are in same area
                if (allNodes[pNeighbor->id].m_localHeight == allNodes[centerId].m_localHeight)
                {
                    // if it's already in the testing queue, don't add it again
                    for (auto v : openSet)
                    {
                        if (v == pNeighbor->id) 
                            return;
                    }

                    // put neighbor in testing queue
                    openSet.push_back(pNeighbor->id);
                }
            }
            if (allNodes[pNeighbor->id].m_localHeight > allNodes[centerId].m_localHeight)
            {
                allNodes[pNeighbor->id].m_isWalkable = false;
            }
        }
    };

    m_areas.emplace_back();

    std::vector<int> openSet;
    openSet.emplace_back(nodeId);

    for (int i = 0; i < openSet.size(); ++i)
    {
        auto* pThisTile = m_pGrid->GetTile(openSet[i]);

        auto* pNorth = m_pGrid->GetNeighborTile(pThisTile,Grid::Direction::North);
        auto* pSouth = m_pGrid->GetNeighborTile(pThisTile,Grid::Direction::South);
        auto* pEast = m_pGrid->GetNeighborTile(pThisTile,Grid::Direction::East);
        auto* pWest = m_pGrid->GetNeighborTile(pThisTile,Grid::Direction::West);

        testNeighbor(pNorth, m_tileNodes, pThisTile->id, openSet);
        testNeighbor(pSouth, m_tileNodes, pThisTile->id, openSet);
        testNeighbor(pEast, m_tileNodes, pThisTile->id, openSet);
        testNeighbor(pWest, m_tileNodes, pThisTile->id, openSet);

        m_areas.back().push_back(pThisTile->id);
        m_tileNodes[pThisTile->id].m_isTested = true;
    }
}

void LocalMap::DrawArea()
{
    static constexpr int textureSize = 16;
    static constexpr int tileSetSize = 3;
    for (int i =0; i< m_tileTextures.size(); ++i)
    {
        E2::Rect srcRect{ m_tileTextures[i].textureId % tileSetSize * textureSize
                         ,m_tileTextures[i].textureId / tileSetSize * textureSize
                         ,textureSize,textureSize };
        DrawTexture(m_tileSets[m_tileTextures[i].tileSetId], &srcRect, &(m_pGrid->GetTile(i)->rect));
    }
    for (auto* pObject : m_grass)
    {
        pObject->Draw();
    }

    for (auto* pObject : m_treesAndBuildings)
    {
        pObject->Draw();
    }
}

void LocalMap::LoadTerrainTexture()
{
    m_tileTextures.resize(m_pGrid->Size());
    SetTileSetId();

    auto tileIsInArea = [](int id,std::vector<int>& area)->bool
    {
        if (id == -1)
        {
            return false;
        }
        for (auto& tileId : area)
        {
            if (id == tileId)
            {
                return true;
            }
        }
        return false;
    };

    auto getTextureIdInTileSet = [](char condition,int xOffset, int yOffset) -> int
    {
        static constexpr int tileSetDimension = 3;
        // 0|1|2
        // 3|4|5
        // 6|7|8

        switch (condition)
        {
        case (0b0000'0110)/* pos = 0 */: return (0 + xOffset) + (0 + yOffset)*tileSetDimension;
        case (0b0000'1110)/* pos = 1 */: return (1 + xOffset) + (0 + yOffset)*tileSetDimension;
        case (0b0000'1010)/* pos = 2 */: return (2 + xOffset) + (0 + yOffset)*tileSetDimension;
        case (0b0000'0111)/* pos = 3 */: return (0 + xOffset) + (1 + yOffset)*tileSetDimension;
        case (0b0000'1111)/* pos = 4 */: return (1 + xOffset) + (1 + yOffset)*tileSetDimension;
        case (0b0000'1011)/* pos = 5 */: return (2 + xOffset) + (1 + yOffset)*tileSetDimension;
        case (0b0000'0101)/* pos = 6 */: return (0 + xOffset) + (2 + yOffset)*tileSetDimension;
        case (0b0000'1101)/* pos = 7 */: return (1 + xOffset) + (2 + yOffset)*tileSetDimension;
        case (0b0000'1001)/* pos = 8 */: return (2 + xOffset) + (2 + yOffset)*tileSetDimension;
        default: /* error */ assert(false); return -1;
        }
    };

    for (auto& area : m_areas)
    {
        for (auto tileId : area)
        {
            //choose a texture for this tile depending on its position in the area

            auto northTile = m_pGrid->GetNeighborTile(m_pGrid->GetTile(tileId),Grid::Direction::North);
            auto southTile = m_pGrid->GetNeighborTile(m_pGrid->GetTile(tileId),Grid::Direction::South);
            auto eastTile = m_pGrid->GetNeighborTile(m_pGrid->GetTile(tileId),Grid::Direction::East);
            auto westTile = m_pGrid->GetNeighborTile(m_pGrid->GetTile(tileId), Grid::Direction::West);

            int northId = (northTile == nullptr) ? -1 : northTile->id;
            int southId = (southTile == nullptr) ? -1 : southTile->id;
            int eastId = (eastTile == nullptr) ? -1 : eastTile->id;
            int westId = (westTile == nullptr) ? -1 : westTile->id;

            //offset because height difference
            //it will not be same because they are in different areas
            int xOffset = 0;
            int yOffset = 0;

            char condition = 0;
            if (tileIsInArea(northId, area))
            {
                condition |= 0b0000'0001;
            }
            else if(northId != -1)// this tile touches other area to the north
            {
                if (!m_tileNodes[northId].m_isWalkable)
                {
                    if (m_tileNodes[northId].m_localHeight > m_tileNodes[tileId].m_localHeight)
                    {
                        ++yOffset;
                    }
                }
            }
            else // this tile touches window to the north
            {
                ++yOffset;
            }

            if (tileIsInArea(southId, area))
            {
                condition |= 0b0000'0010;
            }
            else if (southId != -1)
            {
                if (!m_tileNodes[southId].m_isWalkable)
                {
                    if (m_tileNodes[southId].m_localHeight > m_tileNodes[tileId].m_localHeight)
                    {
                        --yOffset;
                    }
                }
            }
            else
            {
                --yOffset;
            }

            if (tileIsInArea(eastId, area))
            {
                condition |= 0b0000'0100;
            }
            else if (eastId != -1)
            {
                if (!m_tileNodes[eastId].m_isWalkable)
                {
                    if (m_tileNodes[eastId].m_localHeight > m_tileNodes[tileId].m_localHeight)
                    {
                        --xOffset;
                    }
                }
            }
            else
            {
                --xOffset;
            }

            if (tileIsInArea(westId, area))
            {
                condition |= 0b0000'1000;
            }
            else if (westId != -1)
            {
                if (!m_tileNodes[westId].m_isWalkable)
                {
                    if (m_tileNodes[westId].m_localHeight > m_tileNodes[tileId].m_localHeight)
                    {
                        ++xOffset;
                    }
                }
            }
            else
            {
                ++xOffset;
            }

            
            m_tileTextures[tileId].textureId = getTextureIdInTileSet(condition, xOffset, yOffset);

            TileSet usingTileSet = TileSet::Undefined;
            switch (m_pGrid->GetTile(tileId)->biomeId)
            {
            case 1:  
            case 2:  
            case 3:  
            case 4:  
            case 5:  //usingTileSet = TileSet::UnderWater; break;
            case 6:  usingTileSet = TileSet::UnderWater0; break;
            case 7:
            case 8:
            case 9:  usingTileSet = TileSet::GrassyTerrain; break;
            case 10: usingTileSet = TileSet::SnowyTerrain; break;
            case 11:
            case 12: usingTileSet = TileSet::GrassyTerrain; break;
            case 13: usingTileSet = TileSet::SnowyTerrain; break;
            case 14: usingTileSet = TileSet::GrassyTerrain; break;
            case 15: 
            case 16: usingTileSet = TileSet::DryTerrain; break;
            case 17: 
            case 18: 
            case 19:
            case 20: usingTileSet = TileSet::RockyTerrain; break;
            case 21: 
            case 22: usingTileSet = TileSet::SnowyTerrain; break;
            default: assert(false && "no such tileset");
            }



            if (usingTileSet == TileSet::UnderWater0)
            {
                if (m_tileNodes[tileId].m_localHeight < 0.2)
                {
                    usingTileSet = TileSet::UnderWater3;
                }
                else if (m_tileNodes[tileId].m_localHeight < 0.3)
                {
                    usingTileSet = TileSet::UnderWater2;
                }
                else if (m_tileNodes[tileId].m_localHeight < 0.4)
                {
                    usingTileSet = TileSet::UnderWater1;
                }
            }

            m_tileTextures[tileId].tileSetId = usingTileSet;
            
            assert(m_tileTextures[tileId].textureId >= 0 && m_tileTextures[tileId].textureId < 9);

            // 0|1|2
            // 3|4|5
            // 6|7|8
            switch (condition)
            {
            case (0b0000'1110)/* pos = 1 */: m_tileNodes[tileId].m_position = 1; m_tileNodes[tileId].m_canOpen = true; break;
            case (0b0000'0111)/* pos = 3 */: m_tileNodes[tileId].m_position = 3; m_tileNodes[tileId].m_canOpen = true; break;
            case (0b0000'1011)/* pos = 5 */: m_tileNodes[tileId].m_position = 5; m_tileNodes[tileId].m_canOpen = true; break;
            case (0b0000'1101)/* pos = 7 */: m_tileNodes[tileId].m_position = 7; m_tileNodes[tileId].m_canOpen = true; break;
            default: break;
            }
        }
    }
}

void LocalMap::LoadTilesetTexture()
{
    auto sameBiome = [this](int thisTileId, int thatTileId)
    {
        if (thatTileId == -1)
        {
            return true;
        }
        if (m_pGrid->GetTile(thisTileId)->type == Tile::Type::Road)
        {
            return m_pGrid->GetTile(thisTileId)->type == m_pGrid->GetTile(thatTileId)->type;
        }
        else
        {
            return m_pGrid->GetTile(thisTileId)->biomeId == m_pGrid->GetTile(thatTileId)->biomeId;
        }
    };

    auto getTextureIdInTileSet = [](char condition, int xOffset, int yOffset) -> int
    {
        static constexpr int tileSetDimension = 3;
        // 0|1|2
        // 3|4|5
        // 6|7|8

        switch (condition)
        {
        case (0b0000'0110)/* pos = 0 */: return (0 + xOffset) + (0 + yOffset) * tileSetDimension;
        case (0b0000'1110)/* pos = 1 */: return (1 + xOffset) + (0 + yOffset) * tileSetDimension;
        case (0b0000'1010)/* pos = 2 */: return (2 + xOffset) + (0 + yOffset) * tileSetDimension;
        case (0b0000'0111)/* pos = 3 */: return (0 + xOffset) + (1 + yOffset) * tileSetDimension;
        case (0b0000'1111)/* pos = 4 */: return (1 + xOffset) + (1 + yOffset) * tileSetDimension;
        case (0b0000'1011)/* pos = 5 */: return (2 + xOffset) + (1 + yOffset) * tileSetDimension;
        case (0b0000'0101)/* pos = 6 */: return (0 + xOffset) + (2 + yOffset) * tileSetDimension;
        case (0b0000'1101)/* pos = 7 */: return (1 + xOffset) + (2 + yOffset) * tileSetDimension;
        case (0b0000'1001)/* pos = 8 */: return (2 + xOffset) + (2 + yOffset) * tileSetDimension;
        default: /* error */ assert(false); return -1;
        }
    };

    for (int i = 0; i< m_tileTextures.size(); ++i)
    {
        auto* pTile = m_pGrid->GetTile(i);

        TileSet newTileSet = TileSet::Undefined;
        bool overrideBelow = false;
        switch (pTile->biomeId)
        {
        case 1:  newTileSet = TileSet::ThickIce; overrideBelow = true; break;
        case 2:  newTileSet = TileSet::ThinIce;  overrideBelow = true; break;

        case 6:  newTileSet = TileSet::Beach; break;

        case 8:  newTileSet = TileSet::Grass; break;
        
        case 12: newTileSet = TileSet::Forest; break;

        case 16: newTileSet = TileSet::Dirt; break;
        case 17: newTileSet = TileSet::Sand; break;

        case 21: newTileSet = TileSet::Ice; break;
        //default: assert(false && "no such tileset");
        }

        if (pTile->type == Tile::Type::Road)
        {
            newTileSet = TileSet::Road;
            overrideBelow = false;
            m_tileNodes[i].m_isWalkable = true;
        }

        if (newTileSet == TileSet::Undefined)
        {
            continue;
        }

        if (!m_tileNodes[i].m_isWalkable)
        {
            if (!overrideBelow)
            {
                continue;
            }
        }

        auto northTile = m_pGrid->GetNeighborTile(m_pGrid->GetTile(i), Grid::Direction::North);
        auto southTile = m_pGrid->GetNeighborTile(m_pGrid->GetTile(i), Grid::Direction::South);
        auto eastTile = m_pGrid->GetNeighborTile(m_pGrid->GetTile(i), Grid::Direction::East);
        auto westTile = m_pGrid->GetNeighborTile(m_pGrid->GetTile(i), Grid::Direction::West);

        int northId = (northTile == nullptr) ? -1 : northTile->id;
        int southId = (southTile == nullptr) ? -1 : southTile->id;
        int eastId = (eastTile == nullptr) ? -1 : eastTile->id;
        int westId = (westTile == nullptr) ? -1 : westTile->id;

        int xOffset = 0;
        int yOffset = 0;
        char condition = 0;
        if (sameBiome(i, northId))
        {
            condition |= 0b0000'0001;
        }
        else if (northId != -1)// this tile touches other area to the north
        {
            if (!m_tileNodes[northId].m_isWalkable)
            {
                if (m_tileNodes[northId].m_localHeight > m_tileNodes[i].m_localHeight)
                ++yOffset;
            }
        }
        else // this tile touches window to the north
        {
            ++yOffset;
        }

        if (sameBiome(i, southId))
        {
            condition |= 0b0000'0010;
        }
        else if (southId != -1)
        {
            if (!m_tileNodes[southId].m_isWalkable)
            {
                if (m_tileNodes[southId].m_localHeight > m_tileNodes[i].m_localHeight)
                --yOffset;
            }
        }
        else 
        {
            --yOffset;
        }

        if (sameBiome(i, eastId))
        {
            condition |= 0b0000'0100;
        }
        else if (eastId != -1)
        {
            if (m_tileNodes[eastId].m_localHeight > m_tileNodes[i].m_localHeight)
            if (!m_tileNodes[eastId].m_isWalkable)
            {
                --xOffset;
            }
        }
        else
        {
            --xOffset;
        }

        if (sameBiome(i, westId))
        {
            condition |= 0b0000'1000;
        }
        else if (westId != -1)
        {
            if (m_tileNodes[westId].m_localHeight > m_tileNodes[i].m_localHeight)
            if (!m_tileNodes[westId].m_isWalkable)
            {
                ++xOffset;
            }
        }
        else
        {
            ++xOffset;
        }
        m_tileTextures[i].tileSetId = newTileSet;
        m_tileTextures[i].textureId = getTextureIdInTileSet(condition,xOffset,yOffset);
    }
}

void LocalMap::BuildTown()
{
    auto testTile = [this](int x, int y, int xOff, int yOff) -> bool
    {
        bool isBad = false;
        for (int b = y; b < y + yOff; ++b)
        {
            for (int a = x; a < x + xOff; ++a)
            {
                auto* pTile = m_pGrid->GetTile(a, b);
                if (!pTile)
                {
                    isBad = true;
                    break;
                }

                if (pTile->type != Tile::Type::Building)
                {
                    isBad = true;
                    break;
                }

                if (!m_tileNodes[pTile->id].m_isWalkable)
                {
                    isBad = true;
                    break;
                }

                if (pTile->biomeId < 6) //TODO
                {
                    isBad = true;
                    break;
                }
            }
            if (isBad)
            {
                break;
            }
        }
        return isBad;
    };

    auto spawnBuilding = [this,&testTile](Region* pRegion, Grid* pGrid)
    {
        //on leaf node
        if (!(pRegion->m_pLeft || pRegion->m_pRight))
        {
            int startX = pRegion->m_innerRegion.first.x;
            int startY = pRegion->m_innerRegion.first.y;
            int endX = pRegion->m_innerRegion.second.x;
            int endY = pRegion->m_innerRegion.second.y;

            std::unordered_map<int, std::vector<BuildingType>> buildableTiles;

            for (int y = startY; y < endY; ++y)
            {
                for (int x = startX; x < endX; ++x)
                {
                    int tileId = pGrid->GetTile(x, y)->id;
                    bool testResualt1 = testTile(x,y,4,4+1); //+1 because the south should be empty to make room for doors
                    bool testResualt2 = testTile(x,y,5,3+1);
                    bool testResualt3 = testTile(x,y,5,4+1);
                    if (!testResualt1)
                    {
                        buildableTiles[tileId].push_back(BuildingType::X4Y4);
                    }
                    if (!testResualt2)
                    {
                        buildableTiles[tileId].push_back(BuildingType::X5Y3);
                    }
                    if (!testResualt3)
                    {
                        buildableTiles[tileId].push_back(BuildingType::X5Y4);
                    }
                }
            }

            if (!buildableTiles.empty())
            {
                //randomly choose a good tile
                auto random_it = std::next(std::begin(buildableTiles), E2::Random(0, buildableTiles.size() - 1));
                //randomly choose a building type
                int buildingChoiceId = (int)E2::Random(0, random_it->second.size() - 1);
                BuildingType buildingChoice = random_it->second[buildingChoiceId];
                m_buildableTiles[random_it->first] = buildingChoice;
            }
        }
    };

    //Find good places for buildings
    for (auto* pRegion : m_rootRegions)
    {
        Region::WalkTheTree(pRegion, spawnBuilding, m_pGrid);
    }

    //Set the tiles underneath unwalkable, find entrance tile for path finding
    for (auto& [tileId, buildingType] : m_buildableTiles)
    {
        auto* pTile = m_pGrid->GetTile(tileId);
        E2::Vector2 pos = { pTile->x, pTile->y };
        E2::Vector2 dimension{};
        E2::Vector2 entrance{};
        switch (buildingType)
        {
        case BuildingType::X4Y4: dimension = { 4, 4 }; entrance = { 1,3 }; break;
        case BuildingType::X5Y3: dimension = { 5, 3 }; entrance = { 3,2 }; break;
        case BuildingType::X5Y4: dimension = { 5, 4 }; entrance = { 1,3 }; break;
        }
        for (int y1 = pos.y; y1 < pos.y + dimension.y; ++y1)
        {
            for (int x1 = pos.x; x1 < pos.x + dimension.x; ++x1)
            {
                auto* pTempTile = m_pGrid->GetTile(x1,y1);
                m_tileNodes[pTempTile->id].m_isWalkable = false;
            }
        }
        //create entrance tile for path finding to build roads in town
        auto* pDoorStep = m_pGrid->GetTile(pos.x+entrance.x,pos.y+entrance.y);
        m_doorSteps.emplace_back(pDoorStep->id);
    }

    //Create buildings
    for (auto& [tileId,buildingType] : m_buildableTiles)
    {
        auto* pTile = m_pGrid->GetTile(tileId);
        int tileSize = pTile->rect.w;
        E2::Vector2 pos = { pTile->rect.x, pTile->rect.y };
        E2::Vector2 dimension{};
        switch (buildingType)
        {
        case BuildingType::X4Y4: dimension = { tileSize * 4,tileSize * 4 }; break;
        case BuildingType::X5Y3: dimension = { tileSize * 5,tileSize * 3 }; break;
        case BuildingType::X5Y4: dimension = { tileSize * 5,tileSize * 4 }; break;
        }
        auto* pBuilding = new Building(buildingType, m_buildingSets[buildingType], pos, dimension);
        m_treesAndBuildings.push_back(pBuilding);
    }
}

// connect the buldings from door to door
void LocalMap::BuildTownRoad()
{
    if (m_doorSteps.size() > 1)
    {
        for (int i = 0; i < m_doorSteps.size(); ++i)
        {
            for (int k = 0; k < m_doorSteps.size(); ++k)
            {
                if (i == k)
                {
                    continue;
                }
                if (m_pathNodes.empty())
                {
                    m_pathNodes = std::vector<PathNode>(m_pGrid->Size());
                }
                else
                {
                    ReFreshPathNodesForPathing();
                }
                FindPath(m_doorSteps[i], m_doorSteps[k]);
            }
        }
    }

    for (auto id : m_roads)
    {
        m_tileTextures[id].tileSetId = TileSet::TownRoad;
        m_tileTextures[id].textureId = 0;
    }
}

void LocalMap::DrawAreaColor()
{

    for (int i = 0; i <m_areas.size(); ++i)
    {
        for (auto tileId : m_areas[i])
        {
            DrawRect(m_pGrid->GetTile(tileId)->rect, m_colors[i]);
        }
    }

    for (int i = 0; i < m_tileNodes.size(); ++i)
    {
        if (!m_tileNodes[i].m_isWalkable)
            DrawRectOutline(m_pGrid->GetTile(i)->rect, E2::Red::kRed);
    }

    for (auto tileId : m_doorSteps)
    {
        auto* pTile = m_pGrid->GetTile(tileId);
        DrawRect(pTile->rect, E2::Blue::kBlue);
    }

    for (auto tileId : m_roads)
    {
        auto* pTile = m_pGrid->GetTile(tileId);
        DrawRectOutline(pTile->rect, E2::Blue::kBlue);
    }

    for (int i = 0; i < m_tileNodes.size(); ++i)
    {
        if (m_tileNodes[i].m_isGrass)
            DrawRectOutline(m_pGrid->GetTile(i)->rect, E2::Green::kGreen);
    }

}

void LocalMap::ConnectAreas()
{
    for (auto& area : m_areas)
    {
        //gather all eadges
        std::vector<int> eadgeTiles;
        for (int i = 0; i < area.size(); ++i)
        {
            if (m_tileNodes[area[i]].m_canOpen)
            {
                eadgeTiles.push_back(area[i]);
            }
        }
        int connections = (int)eadgeTiles.size() / 40; // Tune number

        for (int j = 0; j < connections; ++j)
        {
            int choice = (int)E2::Random(0, eadgeTiles.size() - 1);
            assert(m_tileNodes[eadgeTiles[choice]].m_position != -1);
            m_tileTextures[eadgeTiles[choice]].textureId = 4;
            m_tileNodes[eadgeTiles[choice]].m_isWalkable = true;
        }
    }
}

void LocalMap::PopulateTrees()
{
    //prepare noise
    std::vector<float> noiseMap = GetPerlinNoiseMap(5.f,15.f);

    auto testTile = [this](int x, int y, int xOff, int yOff) -> bool
    {
        bool isGood = false;
        for (int b = y; b < y + yOff; ++b)
        {
            for (int a = x; a < x + xOff; ++a)
            {
                auto* pTile = m_pGrid->GetTile(a, b);
                if (!pTile || pTile->type == Tile::Type::City
                    || pTile->type == Tile::Type::Road
                    || !m_tileNodes[pTile->id].m_isWalkable
                   )
                {
                    isGood = false;
                    break;
                }
                else
                {
                    isGood = true;
                }
            }
            if (!isGood)
            {
                break;
            }
        }
        return isGood;
    };

    for (int i = 0; i < m_pGrid->Size(); ++i)
    {
        auto* pTile = m_pGrid->GetTile(i);
        // if all tiles with in range is good, maybe plant the tree
        if (testTile(pTile->x, pTile->y, 3, 3))
        {
            // test the tile at the root
            //XXX
            //XXX
            //XOX O = root
            //XXX
            auto* pRootTile = m_pGrid->GetTile(pTile->x + 1, pTile->y + 2);
            if (noiseMap[pRootTile->id] < 0.1f)
            {
                continue;
            }

            auto biomeType = pRootTile->biomeId;
            TreeType treeType = TreeType::None;
            switch (biomeType)
            {
            case 7:     
            case 8:
            case 9:  treeType = TreeType::Green; break;
            case 10: treeType = TreeType::Cold; break;
            case 11: treeType = TreeType::Rain; break;
            case 12: treeType = TreeType::Green; break;
            case 13: treeType = TreeType::HalfSnow; break;
            case 14: treeType = TreeType::Cold; break;
            case 15: treeType = TreeType::Yellow; break;
            case 16: treeType = TreeType::Dead; break;
            case 22: treeType = TreeType::HalfSnow; break;
            default: break;
            }

            if (treeType != TreeType::None)
            {
                auto tileSize = pTile->rect.w;
                E2::Vector2 pos = { pTile->rect.x, pTile->rect.y - tileSize};
                E2::Vector2 dimension = { tileSize * 3,tileSize * 4 };
                auto* pTree = new Tree(m_treeSets[treeType], pos, dimension);
                m_treesAndBuildings.push_back(pTree);

                //block out the space
                for (auto b = pTile->y; b < pTile->y + 3; ++b)
                {
                    for (auto a = pTile->x; a < pTile->x + 3; ++a)
                    {
                        auto* pTempTile = m_pGrid->GetTile(a, b);
                        m_tileNodes[pTempTile->id].m_isWalkable = false;
                    }
                }

                //isometric

                int layers = (int)(m_tileNodes[pRootTile->id].m_localHeight * 10.f) + 1;
                int x1 = (pRootTile->x * 16) - (pRootTile->y * 16);
                int y1 = (pRootTile->x * 8) + (pRootTile->y * 8);

                int yOffset = (layers+1) * 16;
                E2::Rect dest{ x1 - 16 + 350 ,y1 - yOffset ,32,48 };

                pTree->CreateIsometric(m_treeTextures[treeType], dest);
            }
        }
    }
}

void LocalMap::PopulateGrass()
{
    // prepare Noise
    std::vector<float> noise = GetPerlinNoiseMap(5.f,15.f);

    for (int i = 0; i < m_pGrid->Size(); ++i)
    {
        auto* pTile = m_pGrid->GetTile(i);
        // if all tiles with in range is good, maybe spawn grass
        if (pTile && pTile->type != Tile::Type::City
            && pTile->type != Tile::Type::Road
            && m_tileNodes[pTile->id].m_isWalkable
            && !m_tileNodes[pTile->id].m_occupied
            && noise[pTile->id] > 0.1f)
        {
            // test the tile 
            auto biomeType = pTile->biomeId;
            GrassType grassType = GrassType::None;
            switch (biomeType)
            {
            case 7:  grassType = GrassType::Green; break;
            case 8:  grassType = GrassType::Tall; break;
            case 9:  grassType = GrassType::Green; break;
            case 10: grassType = GrassType::Cold; break;
            case 11: grassType = GrassType::Rain; break;
            case 12: grassType = GrassType::Rain; break;
            case 13: grassType = GrassType::HalfSnow; break;
            case 14: grassType = GrassType::Cold; break;
            case 15: grassType = GrassType::Yellow; break;
            case 22: grassType = GrassType::HalfSnow; break;
            default: break;
            }

            if (grassType != GrassType::None)
            {
                auto tileSize = pTile->rect.w;
                E2::Vector2 pos = { pTile->rect.x, pTile->rect.y };
                E2::Vector2 dimension = { tileSize, tileSize};
                auto* pGrass = new Grass(m_grassSets[grassType], pos, dimension);
                m_grass.push_back(pGrass);

                m_tileNodes[pTile->id].m_occupied = true;
                m_tileNodes[pTile->id].m_isGrass = true;
                m_tileNodes[pTile->id].m_grassId = m_grass.size() - 1;

                //isometric
                int layers = (int)(m_tileNodes[pTile->id].m_localHeight * 10.f) + 1;
                int x1 = (pTile->x * 16) - (pTile->y * 16);
                int y1 = (pTile->x * 8) + (pTile->y * 8);

                int yOffset = (layers ) * 16;
                E2::Rect dest{ x1 - 16 + 350 ,y1 - yOffset ,32,32 };

                pGrass->CreateIsometric(m_grassTextures[grassType], dest);
            }
        }
    }
}

void LocalMap::OnNotify(E2::Event evt)
{
    if (!m_pGrid || m_pGrid->Size() == 0)
    {
        return;
    }

    int x = evt.m_mouseEvent.x;
    int y = evt.m_mouseEvent.y;
    int tileSize = m_pGrid->GetTileSize().x;
    auto* pTile = m_pGrid->GetTile(x/tileSize,y/tileSize);

    //right click
    if (evt.m_mouseEvent.button == 3)
    {
        std::cout << "*************** Clicked Tile: ***************\n";
        std::cout << "Index = " << pTile->id << "\n";
        std::cout << "x = " << pTile->x << ", y = " << pTile->y << "\n";
        std::cout << "xCoord = " << pTile->rect.x << ", yCoord = " << pTile->rect.x << "\n";
        std::cout << "HeightNoise = " << pTile->heightNoise << "\n";
        std::cout << "Temperature = " << pTile->rawTemperature << "\n";
        std::cout << "Biome ID = " << pTile->biomeId << "\n";
        std::cout << "Terrain Height = " << m_tileNodes[pTile->id].m_localHeight << "\n";
        std::cout << "Tile ID = " << m_tileNodes[pTile->id].m_tileId << "\n";
        std::cout << "Tile Relative Position = " << (int) m_tileNodes[pTile->id].m_pos << "\n";
        std::cout << "*********************************************\n";
    }

    //middle click
    if (evt.m_mouseEvent.button == 2)
    {
        //MapGenerator::Get().SpawnPlayer(pTile);
    }
}

void LocalMap::SetTileSetId()
{
    (m_tileSets[TileSet::GrassyTerrain]) = GetEngine().CreateTexture("Assets/Tile/Terrain/Grassy.bmp");
    (m_tileSets[TileSet::SnowyTerrain])= GetEngine().CreateTexture("Assets/Tile/Terrain/Snowy.bmp");
    (m_tileSets[TileSet::RockyTerrain])= GetEngine().CreateTexture("Assets/Tile/Terrain/Rocky.bmp");
    (m_tileSets[TileSet::DryTerrain])= GetEngine().CreateTexture("Assets/Tile/Terrain/Dry.bmp");
    (m_tileSets[TileSet::UnderWater0])= GetEngine().CreateTexture("Assets/Tile/Terrain/UnderWater-0.bmp");
    (m_tileSets[TileSet::UnderWater1])= GetEngine().CreateTexture("Assets/Tile/Terrain/UnderWater-1.bmp");
    (m_tileSets[TileSet::UnderWater2])= GetEngine().CreateTexture("Assets/Tile/Terrain/UnderWater-2.bmp");
    (m_tileSets[TileSet::UnderWater3])= GetEngine().CreateTexture("Assets/Tile/Terrain/UnderWater-3.bmp");
    (m_tileSets[TileSet::Beach])= GetEngine().CreateTexture("Assets/Tile/Terrain/Beach.bmp");
    (m_tileSets[TileSet::Grass])= GetEngine().CreateTexture("Assets/Tile/Terrain/Grass.bmp");
    (m_tileSets[TileSet::Dirt])= GetEngine().CreateTexture("Assets/Tile/Terrain/Dirt.bmp");
    (m_tileSets[TileSet::Forest])= GetEngine().CreateTexture("Assets/Tile/Terrain/Forest.bmp");
    (m_tileSets[TileSet::Ice])= GetEngine().CreateTexture("Assets/Tile/Terrain/Ice.bmp");
    (m_tileSets[TileSet::Sand])= GetEngine().CreateTexture("Assets/Tile/Terrain/Sand.bmp");
    (m_tileSets[TileSet::ThinIce])= GetEngine().CreateTexture("Assets/Tile/Terrain/ThinIce.bmp");
    (m_tileSets[TileSet::ThickIce])= GetEngine().CreateTexture("Assets/Tile/Terrain/ThickIce.bmp");
    (m_tileSets[TileSet::Road])= GetEngine().CreateTexture("Assets/Tile/Terrain/Road1.bmp");
    (m_tileSets[TileSet::TownRoad])= GetEngine().CreateTexture("Assets/Tile/Terrain/TownRoad.bmp");

    (m_buildingSets[BuildingType::X4Y4]) = GetEngine().CreateTexture("Assets/Building/HouseX4Y4.bmp");
    (m_buildingSets[BuildingType::X5Y3]) = GetEngine().CreateTexture("Assets/Building/HouseX5Y3.png");
    (m_buildingSets[BuildingType::X5Y4]) = GetEngine().CreateTexture("Assets/Building/HouseX5Y4.png");
    
    (m_treeSets[TreeType::Cold]) = GetEngine().CreateTexture("Assets/Tree/ColdTree.png");
    (m_treeSets[TreeType::Green]) = GetEngine().CreateTexture("Assets/Tree/GreenTree.png");
    (m_treeSets[TreeType::Yellow]) = GetEngine().CreateTexture("Assets/Tree/YellowTree.png");
    (m_treeSets[TreeType::Rain]) = GetEngine().CreateTexture("Assets/Tree/RainTree.png");
    (m_treeSets[TreeType::HalfSnow]) = GetEngine().CreateTexture("Assets/Tree/HalfSnowTree.png");
    (m_treeSets[TreeType::Dead]) = GetEngine().CreateTexture("Assets/Tree/DeadTree.png");

    (m_grassSets[GrassType::Cold]) = GetEngine().CreateTexture("Assets/Grass/ColdGrass.png");
    (m_grassSets[GrassType::Green]) = GetEngine().CreateTexture("Assets/Grass/GreenGrass.png");
    (m_grassSets[GrassType::Rain]) = GetEngine().CreateTexture("Assets/Grass/RainGrass.png");
    (m_grassSets[GrassType::HalfSnow]) = GetEngine().CreateTexture("Assets/Grass/HalfSnowGrass.png");
    (m_grassSets[GrassType::Yellow]) = GetEngine().CreateTexture("Assets/Grass/YellowGrass.png");
    (m_grassSets[GrassType::Tall]) = GetEngine().CreateTexture("Assets/Grass/TallGrass.png");
}

void LocalMap::FindPath(PathNodeId start, PathNodeId end)
{
    std::vector<PathNodeId> testedNodeId;
    Grid::Direction currentDir = Grid::Direction::South;

    testedNodeId.push_back(start);
    testedNodeId.push_back(end);

    auto comp = [this](TileId left, TileId right)
    {
        return m_pathNodes[left].m_fScore > m_pathNodes[right].m_fScore;
    };

    auto getGScore = [this,&currentDir](PathNodeId current, PathNodeId start, PathNodeId end, Grid::Direction newDir)
    {
        if (!m_tileNodes[current].m_isWalkable)
        {
            if (m_tileNodes[current].m_canOpen)
            {
                return 1000;
            }
            else
            {
                return 100000;
            }
        }
        else
        {
            if (m_tileNodes[current].m_isRoad)
            {
                return 0;
            }
            else
            {
                return 100;
            }
        }
    };

    auto heuristic = [this](PathNodeId current, PathNodeId end)
    {
        auto deltaX = m_pGrid->GetTile(current)->x - m_pGrid->GetTile(end)->x;
        auto deltaY = m_pGrid->GetTile(current)->y - m_pGrid->GetTile(end)->y;
        return deltaX * deltaX + deltaY * deltaY;
    };

    auto findId = [](std::vector<PathNodeId>& container, PathNodeId inId)->bool
    {
        for (auto& id : container)
        {
            if (id == inId)
            {
                return true;
            }
        }
        return false;
    };

    std::priority_queue<PathNodeId, std::vector<PathNodeId>, decltype(comp)> openSet(comp);
    std::vector<PathNodeId> closeSet;

    auto testNeighbor = [this, &findId, &closeSet, &heuristic, &getGScore, &testedNodeId, &openSet]
    (Grid::Direction newDir, PathNodeId neighborNodeId, PathNodeId currentNodeId, PathNodeId startNodeId, PathNodeId endNodeId)->bool
    {
        auto* pNeighborTile = m_pGrid->GetTile(neighborNodeId);
        assert(pNeighborTile);
        // if this neighbor is closed
        if (!findId(closeSet, neighborNodeId))
        {
            float gNew = 0.f;
            float hNew = 0.f;
            float fNew = 0.f;
            
            gNew = m_pathNodes[currentNodeId].m_gScore + getGScore(currentNodeId, startNodeId, endNodeId, newDir);
            hNew = (float)heuristic(currentNodeId, endNodeId);
            fNew = gNew + hNew;

            //if tested, relax
            if (findId(testedNodeId, neighborNodeId))
            {
                if (m_pathNodes[neighborNodeId].m_fScore > fNew)
                {
                    m_pathNodes[neighborNodeId].m_gScore = gNew;
                    m_pathNodes[neighborNodeId].m_fScore = fNew;
                    m_pathNodes[neighborNodeId].m_cameFrom = currentNodeId;
                    openSet.push(neighborNodeId);
                    return true;
                }
            }
            //not tested yet
            else
            {
                testedNodeId.push_back(neighborNodeId);
                m_pathNodes[neighborNodeId].m_gScore = gNew;
                m_pathNodes[neighborNodeId].m_fScore = fNew;
                m_pathNodes[neighborNodeId].m_cameFrom = currentNodeId;
                openSet.push(neighborNodeId);
            }
        }
        return false;
    };

    m_pathNodes[start].m_gScore = 0;
    m_pathNodes[start].m_fScore = (float)heuristic(start, end);
    openSet.push(start);

    while (!(openSet.empty() || openSet.top() == end))
    {
        auto current = openSet.top();
        closeSet.push_back(current);
        openSet.pop();
        // find neighbors
        auto* pCurrentTile = m_pGrid->GetTile(current);
        auto* pNorthTile = m_pGrid->GetNeighborTile(pCurrentTile, Grid::Direction::North);
        if (pNorthTile)
        {
            if (testNeighbor(Grid::Direction::North,pNorthTile->id, current, start, end))
            {
                currentDir = Grid::Direction::North;
            }
        }

        auto* pSouthTile = m_pGrid->GetNeighborTile(pCurrentTile, Grid::Direction::South);
        if (pSouthTile)
        {
            if (testNeighbor(Grid::Direction::South, pSouthTile->id, current, start, end))
            {
                currentDir = Grid::Direction::South;
            }
        }

        auto* pEastTile = m_pGrid->GetNeighborTile(pCurrentTile, Grid::Direction::East);
        if (pEastTile)
        {
            if (testNeighbor(Grid::Direction::East, pEastTile->id, current, start, end))
            {
                currentDir = Grid::Direction::East;
            }
        }

        auto* pWestTile = m_pGrid->GetNeighborTile(pCurrentTile, Grid::Direction::West);
        if (pWestTile)
        {
            if (testNeighbor(Grid::Direction::West, pWestTile->id, current, start, end))
            {
                currentDir = Grid::Direction::West;
            }
        }
    }

    // reconstruct path, it is reversed but it's ok
    auto nodeId = openSet.top();
    while (nodeId != -1)
    {
        m_tileNodes[nodeId].m_isRoad = true;
        m_pGrid->GetTile(nodeId)->type = Tile::Type::Road;
        m_roads.push_back(nodeId);
        nodeId = m_pathNodes[nodeId].m_cameFrom;
    }
}

void LocalMap::LoadIsometricTexture()
{
    //GetEngine().LoadFile("Assets/Isometric/Block/b_dirt.png");
    //Isometrics

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

    m_grassTextures[GrassType::Cold]= GetEngine().CreateTexture("Assets/Isometric/Element/e_tundraGrass.png");
    m_grassTextures[GrassType::Green]= GetEngine().CreateTexture("Assets/Isometric/Element/e_grass.png");
    m_grassTextures[GrassType::HalfSnow]= GetEngine().CreateTexture("Assets/Isometric/Element/e_snowGrass.png");
    m_grassTextures[GrassType::Rain]= GetEngine().CreateTexture("Assets/Isometric/Element/e_forestGrass.png");
    m_grassTextures[GrassType::Tall]= GetEngine().CreateTexture("Assets/Isometric/Element/e_tallGrass.png");
    m_grassTextures[GrassType::Yellow]= GetEngine().CreateTexture("Assets/Isometric/Element/e_dryGrass.png");

    m_treeTextures[TreeType::Cold] = GetEngine().CreateTexture("Assets/Isometric/Element/e_northTree.png");
    m_treeTextures[TreeType::Dead] = GetEngine().CreateTexture("Assets/Isometric/Element/e_cactusTree.png");
    m_treeTextures[TreeType::Green] = GetEngine().CreateTexture("Assets/Isometric/Element/e_tree.png");
    m_treeTextures[TreeType::HalfSnow] = GetEngine().CreateTexture("Assets/Isometric/Element/e_firTree.png");
    m_treeTextures[TreeType::Rain] = GetEngine().CreateTexture("Assets/Isometric/Element/e_tropicalTree.png");
    m_treeTextures[TreeType::Yellow] = GetEngine().CreateTexture("Assets/Isometric/Element/e_yellowTree.png");
}

void LocalMap::DrawTerrainIsometric()
{
    constexpr int seaLevel = 4;

    for (int i = 0; i < m_pGrid->Size(); ++i)
    {
        auto* pTile = m_pGrid->GetTile(i);

        int layers = (int)(m_tileNodes[pTile->id].m_localHeight * 10.f) + 1;
        int x1 = (pTile->x * 16) - (pTile->y * 16);
        int y1 = (pTile->x * 8) + (pTile->y * 8);

        BlockType blockType = GetBlockTypeFromBiome(pTile->biomeId);
        SurfaceType surfaceType = GetSurfaceTypeFromBiome(pTile->biomeId);

        int position = (int)(m_tileNodes[i].m_pos);
        for (int j = 0; j < layers; ++j)
        {
            int yOffset = j * 16;
            E2::Rect dest{ x1 - 16 + 350 ,y1 - yOffset ,32,32 };
            DrawTexture(m_blockTextures[blockType][position], nullptr, &dest);

            //if it is the top layer, draw the surface, if avaliable 
            if (surfaceType != SurfaceType::None && j == layers - 1 && j >= seaLevel)
            {
                DrawTexture(m_surfaceTextures[surfaceType][position], nullptr, &dest);
            }
        }

        //sea level = 4
        if (layers <= seaLevel)
        {
            if (surfaceType != SurfaceType::Ice && surfaceType != SurfaceType::Snow)
            {
                surfaceType = SurfaceType::Water;
            }

            int waterOffset = 4 * 16;
            E2::Rect water{ x1 - 16 + 350 ,y1 - waterOffset ,32,32 };
            DrawTexture(m_surfaceTextures[surfaceType][position], nullptr, &water);
        }

        //map element
        int grassId = m_tileNodes[i].m_grassId;
        if (grassId >= 0)
        {
            static_cast<Grass*>(m_grass[grassId])->DrawIsometric();
        }

    }

    //TODO: separate trees and buildings
    for (auto* pObject : m_treesAndBuildings)
    {
        auto* p = dynamic_cast<Tree*>(pObject);
        if (p)
        {
            p->DrawIsometric();
        }
    }
   
}

void LocalMap::GetTilePositionTest()
{
    //figure out the position of each tile

    auto tileIsInArea = [](int id, std::vector<int>& area)->bool
    {
        if (id == -1)
        {
            return false;
        }
        for (auto& tileId : area)
        {
            if (id == tileId)
            {
                return true;
            }
        }
        return false;
    };

    auto getTilePostion = [](char condition, int xOffset, int yOffset) -> int
    {
        static constexpr int tileSetDimension = 3;
        // 0|1|2
        // 3|4|5
        // 6|7|8

        switch (condition)
        {
        case (0b0000'0110)/* pos = 0 */: return (0 + xOffset) + (0 + yOffset) * tileSetDimension;
        case (0b0000'1110)/* pos = 1 */: return (1 + xOffset) + (0 + yOffset) * tileSetDimension;
        case (0b0000'1010)/* pos = 2 */: return (2 + xOffset) + (0 + yOffset) * tileSetDimension;
        case (0b0000'0111)/* pos = 3 */: return (0 + xOffset) + (1 + yOffset) * tileSetDimension;
        case (0b0000'1111)/* pos = 4 */: return (1 + xOffset) + (1 + yOffset) * tileSetDimension;
        case (0b0000'1011)/* pos = 5 */: return (2 + xOffset) + (1 + yOffset) * tileSetDimension;
        case (0b0000'0101)/* pos = 6 */: return (0 + xOffset) + (2 + yOffset) * tileSetDimension;
        case (0b0000'1101)/* pos = 7 */: return (1 + xOffset) + (2 + yOffset) * tileSetDimension;
        case (0b0000'1001)/* pos = 8 */: return (2 + xOffset) + (2 + yOffset) * tileSetDimension;
        default: /* error */ assert(false); return -1;
        }
    };

    for (auto& area : m_areas)
    {
        for (auto tileId : area)
        {
            //choose a texture for this tile depending on its position in the area

            auto northTile = m_pGrid->GetNeighborTile(m_pGrid->GetTile(tileId), Grid::Direction::North);
            auto southTile = m_pGrid->GetNeighborTile(m_pGrid->GetTile(tileId), Grid::Direction::South);
            auto eastTile = m_pGrid->GetNeighborTile(m_pGrid->GetTile(tileId), Grid::Direction::East);
            auto westTile = m_pGrid->GetNeighborTile(m_pGrid->GetTile(tileId), Grid::Direction::West);

            int northId = (northTile == nullptr) ? -1 : northTile->id;
            int southId = (southTile == nullptr) ? -1 : southTile->id;
            int eastId = (eastTile == nullptr) ? -1 : eastTile->id;
            int westId = (westTile == nullptr) ? -1 : westTile->id;

            //offset because height difference
            //it will not be same because they are in different areas
            int xOffset = 0;
            int yOffset = 0;

            char condition = 0;
            if (tileIsInArea(northId, area))
            {
                condition |= 0b0000'0001;
            }
            else if (northId != -1)// this tile touches other area to the north
            {
                if (m_tileNodes[northId].m_localHeight > m_tileNodes[tileId].m_localHeight)
                {
                    ++yOffset;
                }
            }

            if (tileIsInArea(southId, area))
            {
                condition |= 0b0000'0010;
            }
            else if (southId != -1)
            {
                if (m_tileNodes[southId].m_localHeight > m_tileNodes[tileId].m_localHeight)
                {
                    --yOffset;
                }
            }

            if (tileIsInArea(eastId, area))
            {
                condition |= 0b0000'0100;
            }
            else if (eastId != -1)
            {
                if (m_tileNodes[eastId].m_localHeight > m_tileNodes[tileId].m_localHeight)
                {
                    --xOffset;
                }
            }

            if (tileIsInArea(westId, area))
            {
                condition |= 0b0000'1000;
            }
            else if (westId != -1)
            {
                if (m_tileNodes[westId].m_localHeight > m_tileNodes[tileId].m_localHeight)
                {
                    ++xOffset;
                }
            }

            int position = getTilePostion(condition, xOffset, yOffset);

            switch (position)
            {
            case 0: m_tileNodes[tileId].m_pos = TilePosition::TopLeft; break;
            case 1: m_tileNodes[tileId].m_pos = TilePosition::Top; break;
            case 2: m_tileNodes[tileId].m_pos = TilePosition::TopRight; break;
            case 3: m_tileNodes[tileId].m_pos = TilePosition::MidLeft; break;
            case 4: m_tileNodes[tileId].m_pos = TilePosition::Mid; break;
            case 5: m_tileNodes[tileId].m_pos = TilePosition::MidRight; break;
            case 6: m_tileNodes[tileId].m_pos = TilePosition::BottomLeft; break;
            case 7: m_tileNodes[tileId].m_pos = TilePosition::Bottom; break;
            case 8: m_tileNodes[tileId].m_pos = TilePosition::BottomRight; break;
            }
        }
    }
}

BlockType LocalMap::GetBlockTypeFromBiome(int id)
{
    BlockType blockType = BlockType::Undefined;
    switch (id)
    {
    case 0: blockType = BlockType::Dirt; break;
    case 1: blockType = BlockType::Dirt; break;
    case 2: blockType = BlockType::Dirt; break;
    case 3: blockType = BlockType::Dirt; break;
    case 4: blockType = BlockType::Dirt; break;
    case 5: blockType = BlockType::Sand; break;
    case 6: blockType = BlockType::Sand; break;
    case 7: blockType = BlockType::Dirt; break;
    case 8: blockType = BlockType::Dirt; break;
    case 9: blockType = BlockType::Dirt; break;
    case 10: blockType = BlockType::Dirt; break;
    case 11: blockType = BlockType::Dirt; break;
    case 12: blockType = BlockType::Dirt; break;
    case 13: blockType = BlockType::Dirt; break;
    case 14: blockType = BlockType::Rock; break;
    case 15: blockType = BlockType::Rock; break;
    case 16: blockType = BlockType::Rock; break;
    case 17: blockType = BlockType::Sand; break;
    case 18: blockType = BlockType::Rock; break;
    case 19: blockType = BlockType::Rock; break;
    case 20: blockType = BlockType::Rock; break;
    case 21: blockType = BlockType::Ice; break;
    case 22: blockType = BlockType::Ice; break;
    default: blockType = BlockType::Undefined; break;
    }
    assert(blockType != BlockType::Undefined); 
    return blockType;
}

SurfaceType LocalMap::GetSurfaceTypeFromBiome(int id)
{
    SurfaceType surfaceType = SurfaceType::Undefined;
    switch (id)
    {
    case 1: surfaceType = SurfaceType::Snow; break;
    case 2: surfaceType = SurfaceType::Ice; break;
    case 3: surfaceType = SurfaceType::Water; break;
    case 4: surfaceType = SurfaceType::Water; break;
    case 5: surfaceType = SurfaceType::Water; break;
    case 6: surfaceType = SurfaceType::None; break;
    case 7: surfaceType = SurfaceType::Grass; break;
    case 8: surfaceType = SurfaceType::Grass; break;
    case 9: surfaceType = SurfaceType::Grass; break;
    case 10: surfaceType = SurfaceType::Snow; break;
    case 11: surfaceType = SurfaceType::Forest; break;
    case 12: surfaceType = SurfaceType::Forest; break;
    case 13: surfaceType = SurfaceType::Snow; break;
    case 14: surfaceType = SurfaceType::Grass; break;
    case 15: surfaceType = SurfaceType::Grass; break;
    case 16: surfaceType = SurfaceType::None; break;
    case 17: surfaceType = SurfaceType::None; break;
    case 18: surfaceType = SurfaceType::None; break;
    case 19: surfaceType = SurfaceType::None; break;
    case 20: surfaceType = SurfaceType::Grass; break;
    case 21: surfaceType = SurfaceType::None; break;
    case 22: surfaceType = SurfaceType::Snow; break;
    default: surfaceType = SurfaceType::Undefined; break;
    }
    assert(surfaceType != SurfaceType::Undefined);
    return surfaceType;
}

void LocalMap::FindCities()
{
    for (int i = 0; i < m_pGrid->Size(); ++i)
    {
        Tile* pTile = m_pGrid->GetTile(i);
        if (IsTileInCity(pTile))
        {
            continue;
        }
        if (pTile->type == Tile::Type::City)
        {
            auto* pStartTile = pTile;
            //find the end tile
            auto* pEastEnd = pTile;
            auto* pNeighborEast = m_pGrid->GetNeighborTile(pTile, Grid::Direction::East);
            while (pNeighborEast
                && (pNeighborEast->type == Tile::Type::City ))
            {
                pEastEnd = pNeighborEast;
                pNeighborEast = m_pGrid->GetNeighborTile(pNeighborEast, Grid::Direction::East);
            }

            auto* pSouthEnd = pTile;
            auto* pNeighborSouth = m_pGrid->GetNeighborTile(pTile, Grid::Direction::South);
            while (pNeighborSouth
                && (pNeighborSouth->type == Tile::Type::City ))
            {
                pSouthEnd = pNeighborSouth;
                pNeighborSouth = m_pGrid->GetNeighborTile(pNeighborSouth, Grid::Direction::South);
            }

            auto* pEndTile = m_pGrid->GetTile(pEastEnd->x, pSouthEnd->y);
            m_cities.push_back({ pStartTile ,pEndTile });
            //return;
        }
    }
}

void LocalMap::SpawnBuildings()
{
    auto spawnBuilding = [](Region* pRegion, Grid* pGrid)
    {
        //if there are sub regions, the innerRegion is the bound of them
        if (pRegion->m_pLeft && pRegion->m_pRight)
        {
            auto& leftRegion = pRegion->m_pLeft->m_innerRegion;
            auto& rightRegion = pRegion->m_pRight->m_innerRegion;
            auto startX = leftRegion.first.x < rightRegion.first.x ? leftRegion.first.x : rightRegion.first.x;
            auto startY = leftRegion.first.y < rightRegion.first.y ? leftRegion.first.y : rightRegion.first.y;
            auto endX = leftRegion.second.x > rightRegion.second.x ? leftRegion.second.x : rightRegion.second.x;
            auto endY = leftRegion.second.y > rightRegion.second.y ? leftRegion.second.y : rightRegion.second.y;
            pRegion->m_innerRegion = { {startX,startY},{endX,endY} };
        }
        //if this is the leaf region, spawn a building here
        else
        {
            auto startX = pRegion->m_region.first.x + (int)E2::Random(1, kBuildingSizeInverse);
            auto startY = pRegion->m_region.first.y + (int)E2::Random(1, kBuildingSizeInverse);
            auto endX = pRegion->m_region.second.x - (int)E2::Random(1, kBuildingSizeInverse);
            auto endY = pRegion->m_region.second.y - (int)E2::Random(1, kBuildingSizeInverse);
            pRegion->m_innerRegion = { {startX,startY},{endX,endY} };

            for (int y = startY; y < endY; ++y)
            {
                for (int x = startX; x < endX; ++x)
                {
                    auto* pTile = pGrid->GetTile(x, y);
                    pTile->type = Tile::Type::Building;
                }
            }
        }
    };

    for (auto* pRegion : m_rootRegions)
    {
        Region::WalkTheTree(pRegion, spawnBuilding, m_pGrid);
    }
}

bool LocalMap::IsTileInCity(Tile* pTile)
{
    for (auto& [start, end] : m_cities)
    {
        if (pTile->x >= start->x && pTile->y >= start->y)
        {
            if (pTile->x <= end->x && pTile->y <= end->y)
            {
                return true;
            }
        }
    }
    return false;
}

std::vector<int> LocalMap::GetSpawnAbleTiles()
{
    std::vector<int> goodTiles;
    for (int i =0; i< m_tileNodes.size(); ++i)
    {
        if (m_tileNodes[i].m_isGrass)
        {
            goodTiles.push_back(i);
        }
    }
    return goodTiles;
}

std::vector<LocalMap::TileId> LocalMap::GetReachableTiles(E2::Vector2f origin, float radius)
{
    int originId = m_pGrid->FindTile(origin);
    if (originId == -1)
    {
        assert(false && "cannot find the tile at location");
        return std::vector<TileId>();
    }

    auto testNeighbor = [this,&originId](Tile* pNeighbor, std::vector<TileNode>& allNodes, int centerId, std::vector<int>& openSet, float distance)
    {
        if (pNeighbor) // quit if neighbor is out of screen
        {
            // quit if neighbor is out of distance
            if(m_pGrid->Distance2(m_pGrid->GetTile(originId), pNeighbor)> distance* distance)
            {
                return;
            }
            
            if (!allNodes[pNeighbor->id].m_isTested) // quit if neighbor is already tested
            {
                // if neighbor is grass
                if (allNodes[pNeighbor->id].m_isGrass)
                {
                    // if it's already in the testing queue, don't add it again
                    for (auto v : openSet)
                    {
                        if (v == pNeighbor->id)
                            return;
                    }

                    // put neighbor in testing queue
                    openSet.push_back(pNeighbor->id);
                }
            }
        }
    };

    ReFreshTileNodesForSearching();
    std::vector<int> output;

    std::vector<int> openSet;
    openSet.emplace_back(originId);

    for (int i = 0; i < openSet.size(); ++i)
    {
        auto* pThisTile = m_pGrid->GetTile(openSet[i]);

        auto* pNorth = m_pGrid->GetNeighborTile(pThisTile, Grid::Direction::North);
        auto* pSouth = m_pGrid->GetNeighborTile(pThisTile, Grid::Direction::South);
        auto* pEast = m_pGrid->GetNeighborTile(pThisTile, Grid::Direction::East);
        auto* pWest = m_pGrid->GetNeighborTile(pThisTile, Grid::Direction::West);

        testNeighbor(pNorth, m_tileNodes, pThisTile->id, openSet, radius);
        testNeighbor(pSouth, m_tileNodes, pThisTile->id, openSet, radius);
        testNeighbor(pEast, m_tileNodes, pThisTile->id, openSet, radius);
        testNeighbor(pWest, m_tileNodes, pThisTile->id, openSet, radius);

        output.push_back(pThisTile->id);
        m_tileNodes[pThisTile->id].m_isTested = true;
    }
    return output;
}

E2::Vector2f LocalMap::GetRandomNearPositionInNav(E2::Vector2f origin, float radius)
{
    auto goodTiles = GetReachableTiles(origin, radius);
    auto choice = E2::Random(0, goodTiles.size()-1);
    auto* pTile = m_pGrid->GetTile(goodTiles[choice]);
    return E2::Vector2f((float)pTile->rect.x, (float)pTile->rect.y);
}

std::vector<E2::Vector2f> LocalMap::BuildPath(E2::Vector2f from, E2::Vector2f to)
{
    if (m_pathNodes.empty())
    {
        m_pathNodes = std::vector<PathNode>(m_pGrid->Size());
    }
    else
    {
        ReFreshPathNodesForPathing();
    }
   
    int start = m_pGrid->FindTile(from);
    int end = m_pGrid->FindTile(to);

    std::vector<PathNodeId> testedNodeId;
    testedNodeId.push_back(start);
    testedNodeId.push_back(end);

    auto comp = [this](TileId left, TileId right)
    {
        return m_pathNodes[left].m_fScore > m_pathNodes[right].m_fScore;
    };

    auto getGScore = [this](PathNodeId current, PathNodeId start, PathNodeId end, Grid::Direction newDir)
    {
        if (m_tileNodes[current].m_isGrass)
        {
            return 0;
        }
        else if(m_tileNodes[current].m_isWalkable)
        {
            return 1;
        }
        else
        {
            return 1000000;
        }
    };

    auto heuristic = [this](PathNodeId current, PathNodeId end)
    {
        auto deltaX = m_pGrid->GetTile(current)->x - m_pGrid->GetTile(end)->x;
        auto deltaY = m_pGrid->GetTile(current)->y - m_pGrid->GetTile(end)->y;
        return deltaX * deltaX + deltaY * deltaY;
    };

    auto findId = [](std::vector<PathNodeId>& container, PathNodeId inId)->bool
    {
        for (auto& id : container)
        {
            if (id == inId)
            {
                return true;
            }
        }
        return false;
    };

    std::priority_queue<PathNodeId, std::vector<PathNodeId>, decltype(comp)> openSet(comp);
    std::vector<PathNodeId> closeSet;

    auto testNeighbor = [this, &findId, &closeSet, &heuristic, &getGScore, &testedNodeId, &openSet]
    (Grid::Direction newDir, PathNodeId neighborNodeId, PathNodeId currentNodeId, PathNodeId startNodeId, PathNodeId endNodeId)->bool
    {
        auto* pNeighborTile = m_pGrid->GetTile(neighborNodeId);
        assert(pNeighborTile);
        // if this neighbor is closed
        if (!findId(closeSet, neighborNodeId))
        {
            float gNew = 0.f;
            float hNew = 0.f;
            float fNew = 0.f;

            gNew = m_pathNodes[currentNodeId].m_gScore + getGScore(currentNodeId, startNodeId, endNodeId, newDir);
            hNew = (float)heuristic(currentNodeId, endNodeId);
            fNew = gNew + hNew;

            //if tested, relax
            if (findId(testedNodeId, neighborNodeId))
            {
                if (m_pathNodes[neighborNodeId].m_fScore > fNew)
                {
                    m_pathNodes[neighborNodeId].m_gScore = gNew;
                    m_pathNodes[neighborNodeId].m_fScore = fNew;
                    m_pathNodes[neighborNodeId].m_cameFrom = currentNodeId;
                    openSet.push(neighborNodeId);
                    return true;
                }
            }
            //not tested yet
            else
            {
                testedNodeId.push_back(neighborNodeId);
                m_pathNodes[neighborNodeId].m_gScore = gNew;
                m_pathNodes[neighborNodeId].m_fScore = fNew;
                m_pathNodes[neighborNodeId].m_cameFrom = currentNodeId;
                openSet.push(neighborNodeId);
            }
        }
        return false;
    };

    m_pathNodes[start].m_gScore = 0;
    m_pathNodes[start].m_fScore = (float)heuristic(start, end);
    openSet.push(start);

    while (!(openSet.empty() || openSet.top() == end))
    {
        auto current = openSet.top();
        closeSet.push_back(current);
        openSet.pop();
        // find neighbors
        auto* pCurrentTile = m_pGrid->GetTile(current);
        auto* pNorthTile = m_pGrid->GetNeighborTile(pCurrentTile, Grid::Direction::North);
        if (pNorthTile)
        {
            testNeighbor(Grid::Direction::North, pNorthTile->id, current, start, end);
        }

        auto* pSouthTile = m_pGrid->GetNeighborTile(pCurrentTile, Grid::Direction::South);
        if (pSouthTile)
        {
            testNeighbor(Grid::Direction::South, pSouthTile->id, current, start, end);
        }

        auto* pEastTile = m_pGrid->GetNeighborTile(pCurrentTile, Grid::Direction::East);
        if (pEastTile)
        {
            testNeighbor(Grid::Direction::East, pEastTile->id, current, start, end);
        }

        auto* pWestTile = m_pGrid->GetNeighborTile(pCurrentTile, Grid::Direction::West);
        if (pWestTile)
        {
            testNeighbor(Grid::Direction::West, pWestTile->id, current, start, end);
        }
    }

    // reconstruct path
    std::vector<E2::Vector2f> path;
    auto nodeId = openSet.top();
    while (nodeId != -1)
    {
        auto* pTile = m_pGrid->GetTile(nodeId);
        path.emplace_back(pTile->rect.x, pTile->rect.y);
        nodeId = m_pathNodes[nodeId].m_cameFrom;
    }
    std::reverse(path.begin(),path.end());
    return path;
}
