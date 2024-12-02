#include "Terraform.h"
#include "WorldMap.h"
#include "Region.h"
#include "LocalMapElementDef.h"

#include <GlobalFunctions.h>
#include <SimpleMath.h>
#include <PerlinNoise.h>
#include <Random.h>
#include <LuaEmbed.h>

#include <assert.h>
#include <queue>
#include <Iostream>

#define DRAW_SEARCH_TOWN 0

void TerraForm::BasicPerlinNoise(NoiseType noiseType, WorldMap* pMap, int octaves, int inputRange, float persistence)
{
    assert(pMap && octaves > 0 && inputRange > 0 && persistence > 0);

    int column = pMap->GetColumnCount();
    int row = pMap->GetRowCount();

    float longSide = (float)((column > row) ? column : row);    //using long side so the noise will not be distorted
    float currentAmplitude = 0;
    float totalAmplitude = 0;
    int currentInputRange = 0;

    //need new seed for each octave, otherwise it will be same noises overlaying together
    size_t* seedForEachOctave = new size_t[octaves];
    for (int i = 0; i < octaves; ++i)
    {
        seedForEachOctave[i] = E2::Rand::Random();
    }

    for (int i = 0; i < pMap->GetTileCount(); ++i)
    {
        Tile* pTile = pMap->GetTile(i);
        float* pNoise = nullptr;

        switch (noiseType)
        {
        case NoiseType::Height: pNoise = &(pTile->m_heightNoise); break;
        case NoiseType::Temperature: pNoise = &(pTile->m_temperatureNoise); break;
        case NoiseType::Tree: pNoise = &(pTile->m_treeNoise); break;
        case NoiseType::Grass: pNoise = &(pTile->m_grassNoise); break;
        default: assert(false && "wrong noise type"); break;
        }

        *pNoise = 0;

        currentAmplitude = 1.f;
        totalAmplitude = 0;
        currentInputRange = inputRange;

        for (int j = 0; j < octaves; ++j)
        {
            float dx = (float)pTile->m_x / longSide;
            float dy = (float)pTile->m_y / longSide;
            float noise = PerlinNoise::Perlin(dx * currentInputRange, dy * currentInputRange, seedForEachOctave[j]);    //[-1,1]
            noise = noise / 2.f + 0.5f;     //[0,1]
            assert(noise >= 0 && noise <= 1.f);
            *pNoise += currentAmplitude * noise;
            totalAmplitude += currentAmplitude;
            currentAmplitude *= persistence;
            currentInputRange *= 2;
        }

        //TODO: is saving the raw noise necessary?
        *pNoise /= totalAmplitude;
        assert(*pNoise >= 0 && *pNoise <= 1.f);
    }
    delete[] seedForEachOctave;
}

//e.g. if the min and max of current noise is 0.3 and 1.6, after stretch the range will be new min and max
void TerraForm::StretchNoise(NoiseType noiseType, WorldMap* pMap, float min, float max)
{

    float minNoise = std::numeric_limits<float>::max();
    float maxNoise = std::numeric_limits<float>::min();
    //find the min and max noise of current map
    for (int i = 0; i < pMap->GetTileCount(); ++i)
    {
        auto* pTile = pMap->GetTile(i);

        float* pNoise = nullptr;
        switch (noiseType)
        {
        case NoiseType::Height: pNoise = &(pTile->m_heightNoise); break;
        case NoiseType::Temperature: pNoise = &(pTile->m_temperatureNoise); break;
        }

        if (*pNoise <= minNoise)
        {
            minNoise = *pNoise;
        }
        else if (*pNoise >= maxNoise)
        {
            maxNoise = *pNoise;
        }
    }
    //convert to fit the new range
    float noiseRange = maxNoise - minNoise;
    for (int i = 0; i < pMap->GetTileCount(); ++i)
    {
        auto* pTile = pMap->GetTile(i);

        float* pNoise = nullptr;
        switch (noiseType)
        {
        case NoiseType::Height: pNoise = &(pTile->m_heightNoise); break;
        case NoiseType::Temperature: pNoise = &(pTile->m_temperatureNoise); break;
        }

        float weight = (*pNoise - minNoise) / noiseRange;
        *pNoise = E2::Lerp(min, max, weight);
    }
}

//basically a lerp and rounding
void TerraForm::FormulateHeight(WorldMap* pMap, int maxHeight)
{
    auto func = [](Tile* pTile, int maxH)
    {
        pTile->m_height = (std::lroundf(pTile->m_heightNoise * (float)maxH));
    };

    pMap->ForEachTile(func,maxHeight);
    pMap->SetMaxHeight(maxHeight);
}

void TerraForm::SplitRegion(Region* pRoot, WorldMap* pMap, int numberOfTimes, int minRegionSize)
{
    Tile* pFirstTile = pMap->GetTile(0);
    Tile* pLastTile = pMap->GetLastTile();
    pRoot->DefineZone(pFirstTile->m_x,pFirstTile->m_y,pLastTile->m_x,pLastTile->m_y);

    for (size_t i = 0; i < numberOfTimes; ++i)
    {
        auto availableRegions = Region::FindSplittableRegions(pRoot);
        // TODO: make weighted random
        auto choice = E2::Rand::Random(0, availableRegions.size() - 1);
        availableRegions[choice]->Split(minRegionSize);
    }
}

void TerraForm::AdjustRegionalHeightNoise(Region* pRoot, WorldMap* pMap)
{
    auto availableRegions = Region::FindSplittableRegions(pRoot);
    for (auto& region : availableRegions)
    {
        auto zone = region->GetZone();
        int startX = zone.first.x;
        int startY = zone.first.y;
        int endX = zone.second.x;
        int endY = zone.second.y;

        //MN: 10: the check uses base 10 log, so the slope is also 10.
        //making a graph on desmo and you'll see.
        auto emptiness = TerraForm::CheckRegionEmptiness(region->TileCount(),(int)pMap->GetTileCount(), 10);
        for (int y = startY; y <= endY; ++y)
        {
            for (int x = startX; x <= endX; ++x)
            {
                Tile* pTile = pMap->GetTile(x, y);
                {
                    auto nx = (float)(pTile->m_x - startX) / (float)(endX - startX) - 0.5f;
                    auto ny = (float)(pTile->m_y - startY) / (float)(endY - startY) - 0.5f;

                    //there are many ways to form an island shaped noise:
                    auto d = std::sqrtf(nx * nx + ny * ny) / std::sqrtf(0.5f);
                    //auto d = 2 * std::max(abs(nx), abs(ny));
                    //auto d = abs(nx) + abs(ny);

                    d = (emptiness + 3 * d) / 4;
                    auto layerNoise = (1 - d + pTile->m_heightNoise) / 2;
                    pTile->m_heightNoise = (layerNoise * 0.7f + pTile->m_heightNoise * 0.3f);
                    pTile->m_heightNoise *= emptiness;
                    pTile->m_heightNoise = E2::SmoothStep(pTile->m_heightNoise);
                }
            }
        }
    }
}

float TerraForm::CheckRegionEmptiness(int zoneTileCount, int worldTileCount, int slope)
{
    // when slope is 1, the chance of getting negative value is too high for small areas
    assert(slope > 1);
    float areaRatio = (float)zoneTileCount / (float)worldTileCount;
    float emptiness = std::log10f(areaRatio) / (float)slope + 1;
    assert(emptiness > 0 && emptiness < 1.f);
    if (emptiness < 0)
    {
        return 0;
    }
    return emptiness;
}

void TerraForm::SmoothNoiseCrossRegion(Region* pRoot, WorldMap* pWorldMap, int smoothDistance)
{
    auto func = [](Region* pRegion, WorldMap* pMap, int smoothDis)
    {
        auto line = pRegion->GetSplitLine();
        if (line.first.x == std::numeric_limits<int>::max())
        {
            return;
        }
        bool lineIsVertical = (line.first.x == line.second.x) ? true : false;

        if (lineIsVertical)
        {
            for (int y = line.first.y; y <= line.second.y; ++y)
            {
                int x0 = line.first.x;
                auto* pTile = pMap->GetTile(x0, y);
                auto* pLeftTile = pMap->GetTile(x0 - 1, y);
                auto* pRightTile = pMap->GetTile(x0 + 1, y);
                pTile->m_heightNoise = (pLeftTile->m_heightNoise + pRightTile->m_heightNoise) / 2.f;

                //TODO: MN
                //smooth n tiles towards the splitline?
                for (int i = x0; i > x0 - smoothDis; --i)
                {
                    float distance = 1.f - (float)(x0 - i) / (float)smoothDis;
                    if (pMap->GetTile(i-1, y)->m_heightNoise > pMap->GetTile(i, y)->m_heightNoise)
                    {
                        float diff = pMap->GetTile(i-1, y)->m_heightNoise - pMap->GetTile(i, y)->m_heightNoise;
                        pMap->GetTile(i-1, y)->m_heightNoise -= diff / 2.f * distance;
                    }
                    else
                    {
                        float diff = pMap->GetTile(i, y)->m_heightNoise - pMap->GetTile(i - 1, y)->m_heightNoise;
                        pMap->GetTile(i - 1, y)->m_heightNoise += diff / 2.f * distance;
                    }
                }

                for (int i = x0; i < x0 + smoothDis; ++i)
                {
                    float distance = 1.f - (float)(i - x0) / (float)smoothDis;
                    if (pMap->GetTile(i + 1, y)->m_heightNoise > pMap->GetTile(i, y)->m_heightNoise)
                    {
                        float diff = pMap->GetTile(i + 1, y)->m_heightNoise - pMap->GetTile(i, y)->m_heightNoise;
                        pMap->GetTile(i + 1, y)->m_heightNoise -= diff / 2.f * distance;
                    }
                    else
                    {
                        float diff = pMap->GetTile(i, y)->m_heightNoise - pMap->GetTile(i + 1, y)->m_heightNoise;
                        pMap->GetTile(i + 1, y)->m_heightNoise += diff / 2.f * distance;
                    }
                }
            }
        }
        else
        {
            for (int x = line.first.x; x <= line.second.x; ++x)
            {
                int y0 = line.first.y;
                auto* pTile = pMap->GetTile(x, y0);
                auto* pUpTile = pMap->GetTile(x, y0 - 1);
                auto* pDownTile = pMap->GetTile(x, y0 + 1);
                pTile->m_heightNoise = (pUpTile->m_heightNoise + pDownTile->m_heightNoise) / 2.f;

                for (int i = y0; i > y0 - smoothDis; --i)
                {
                    float distance = 1.f - (float)(y0 - i) / (float)smoothDis;
                    if (pMap->GetTile(x, i-1)->m_heightNoise > pMap->GetTile(x, i)->m_heightNoise)
                    {
                        float diff = pMap->GetTile(x, i-1)->m_heightNoise - pMap->GetTile(x, i)->m_heightNoise;
                        pMap->GetTile(x, i-1)->m_heightNoise -= diff / 2.f * distance;
                    }
                    else
                    {
                        float diff = pMap->GetTile(x, i)->m_heightNoise - pMap->GetTile(x, i-1)->m_heightNoise;
                        pMap->GetTile(x, i-1)->m_heightNoise += diff / 2.f * distance;
                    }
                }

                for (int i = y0; i < y0 + smoothDis; ++i)
                {
                    float distance = 1.f - (float)(i - y0) / (float)smoothDis;
                    if (pMap->GetTile(x, i + 1)->m_heightNoise > pMap->GetTile(x, i)->m_heightNoise)
                    {
                        float diff = pMap->GetTile(x, i + 1)->m_heightNoise - pMap->GetTile(x, i)->m_heightNoise;
                        pMap->GetTile(x, i + 1)->m_heightNoise -= diff / 2.f * distance;
                    }
                    else
                    {
                        float diff = pMap->GetTile(x, i)->m_heightNoise - pMap->GetTile(x, i + 1)->m_heightNoise;
                        pMap->GetTile(x, i + 1)->m_heightNoise += diff / 2.f * distance;
                    }
                }
            }
        }
    };
    Region::WalkTheTree(pRoot, func, pWorldMap,smoothDistance);
}

void TerraForm::AdjustTemperatureNoise(WorldMap* pMap, int equator)
{
    auto func = [](Tile* pTile, int eq)
    {
        //temperature
        //the weight is the distance to the equator
        float weight = 0;
        if (pTile->m_y < eq)
        {
            //north hemisphere
            weight = (float)pTile->m_y / (float)eq;
        }
        else
        {
            //south hemisphere
            weight = 2.f - ((float)pTile->m_y / (float)eq);
        }
        weight = E2::SmoothStep(weight);
        pTile->m_temperatureNoise *= weight;
    };
    pMap->ForEachTile(func, equator);
}

//TODO:
void TerraForm::SmoothTemperature(WorldMap* pMap)
{


}

void TerraForm::FindTileSetOffset(unsigned char pos, int& xOffset, int& yOffset)
{
    //   0  1  2
    // 0 NW N NE
    // 1 W  C  E
    // 2 SW S SE
    switch (pos)
    {
        //flags0000'SNWE
    case 0b0000'0000: /*C*/  xOffset += 1; yOffset += 1; break;
    case 0b0000'0001: /*E*/  xOffset += 2; yOffset += 1; break;
    case 0b0000'0010: /*W*/  xOffset += 0; yOffset += 1; break;
    case 0b0000'0100: /*N*/  xOffset += 1; yOffset += 0; break;
    case 0b0000'1000: /*S*/  xOffset += 1; yOffset += 2; break;
    case 0b0000'0101: /*NE*/ xOffset += 2; yOffset += 0; break;
    case 0b0000'1001: /*SE*/ xOffset += 2; yOffset += 2; break;
    case 0b0000'0110: /*NW*/ xOffset += 0; yOffset += 0; break;
    case 0b0000'1010: /*SW*/ xOffset += 0; yOffset += 2; break;
    default: assert(false && "tileset position error"); break;
    }
}

//check surrounding tiles to figure out what position of block should a tile use
void TerraForm::FindBlockId(WorldMap* pMap)
{
    auto testTile = [](WorldMap* pMap, int tileId, WorldMap::Direction dir, uint8_t& position,int& xOffset, int& yOffset)
    {
        Tile* pTile = pMap->GetTile(tileId);
        Tile* pNeighbor = pMap->GetNeighborTile(tileId, dir);
        if (!(pNeighbor && pTile))
            return;

        if (pTile->m_height == pNeighbor->m_height)
            return;

        switch (dir)
        {
        case WorldMap::Direction::E:
        {
            position |= 0b0000'0001;
            xOffset -= (pTile->m_height < pNeighbor->m_height ? 1 : 0);
            break;
        }
        case WorldMap::Direction::W: 
        {
            position |= 0b0000'0010;
            xOffset += (pTile->m_height < pNeighbor->m_height ? 1 : 0);
            break;
        }
        case WorldMap::Direction::N: 
        {
            position |= 0b0000'0100;
            yOffset += (pTile->m_height < pNeighbor->m_height ? 1 : 0);
            break;
        }
        case WorldMap::Direction::S: 
        {
            position |= 0b0000'1000;
            yOffset -= (pTile->m_height < pNeighbor->m_height ? 1 : 0);
            break;
        }
        }
    };

    for (int i = 0; i < pMap->GetTileCount(); ++i)
    {
        uint8_t position = 0b0000'0000; //flags: 0000'SNWE
        int xOffset = 0;
        int yOffset = 0;


        testTile(pMap, i, WorldMap::Direction::E, position, xOffset, yOffset);
        testTile(pMap, i, WorldMap::Direction::W, position, xOffset, yOffset);
        testTile(pMap, i, WorldMap::Direction::N, position, xOffset, yOffset);
        testTile(pMap, i, WorldMap::Direction::S, position, xOffset, yOffset);

        FindTileSetOffset(position, xOffset, yOffset);

        //   0  1  2
        // 0 0  1  2
        // 1 3  4  5
        // 2 6  7  8
        pMap->GetTile(i)->m_blockId = xOffset + yOffset * 3; //3 elements per row
    }
}

//terrain differeces determine offset
//biome and terrain differences determine position
void TerraForm::FindSurfaceId(WorldMap* pMap)
{
    auto testTile = [](WorldMap* pMap, int tileId, WorldMap::Direction dir, uint8_t& position, int& xOffset, int& yOffset)
    {
        Tile* pTile = pMap->GetTile(tileId);
        Tile* pNeighbor = pMap->GetNeighborTile(tileId, dir);
        if (!(pNeighbor && pTile))
            return;

        bool isBiomeDifferent = (pTile->m_biomeId != pNeighbor->m_biomeId);
        bool isTerrainDifferent = (pTile->m_height != pNeighbor->m_height);

        if (!isBiomeDifferent && !isTerrainDifferent)
            return;

        switch (dir)
        {
        case WorldMap::Direction::E:
        {
            if (isBiomeDifferent || isTerrainDifferent)
            {
                position ^= 0b0000'0001;
            }
            xOffset -= (pTile->m_height < pNeighbor->m_height ? 1 : 0);
            break;
        }
        case WorldMap::Direction::W: 
        {
            if (isBiomeDifferent || isTerrainDifferent)
            {
                position ^= 0b0000'0010;
            }
            xOffset += (pTile->m_height < pNeighbor->m_height ? 1 : 0);
            break;
        }
        case WorldMap::Direction::N:
        {
            if (isBiomeDifferent || isTerrainDifferent)
            {
                position ^= 0b0000'0100;
            }
            yOffset += (pTile->m_height < pNeighbor->m_height ? 1 : 0);
            break;
        }
        case WorldMap::Direction::S:
        {
            if (isBiomeDifferent || isTerrainDifferent)
            {
                position ^= 0b0000'1000;
            }
            yOffset -= (pTile->m_height < pNeighbor->m_height ? 1 : 0);
            break;
        }
        }
    };

    for (int i = 0; i < pMap->GetTileCount(); ++i)
    {
        uint8_t position = 0b0000'0000; //flags: 0000'SNWE
        int xOffset = 0;
        int yOffset = 0;

        testTile(pMap, i, WorldMap::Direction::E, position, xOffset, yOffset);
        testTile(pMap, i, WorldMap::Direction::W, position, xOffset, yOffset);
        testTile(pMap, i, WorldMap::Direction::N, position, xOffset, yOffset);
        testTile(pMap, i, WorldMap::Direction::S, position, xOffset, yOffset);

        FindTileSetOffset(position,xOffset,yOffset);
        //   0  1  2
        // 0 0  1  2
        // 1 3  4  5
        // 2 6  7  8
        pMap->GetTile(i)->m_surfaceId = xOffset + yOffset * 3;
    }
}

void TerraForm::FindTownRoadId(WorldMap* pMap)
{
    auto findRoadPosition = [](Tile* pTile, WorldMap* pMap)
    {
        uint8_t position = 0;
        if (pTile->m_isTownRoad)
        {
            position |= pMap->GetNeighborTile(pTile->m_index, WorldMap::Direction::E)->m_isTownRoad ? 0b0000'0001 : 0b0000'0000;
            position |= pMap->GetNeighborTile(pTile->m_index, WorldMap::Direction::W)->m_isTownRoad ? 0b0000'0010 : 0b0000'0000;
            position |= pMap->GetNeighborTile(pTile->m_index, WorldMap::Direction::N)->m_isTownRoad ? 0b0000'0100 : 0b0000'0000;
            position |= pMap->GetNeighborTile(pTile->m_index, WorldMap::Direction::S)->m_isTownRoad ? 0b0000'1000 : 0b0000'0000;

            //S N W E
            //
            switch (position)
            {
            case 0b0000'0001: pTile->m_townRoadId = 1; break;
            case 0b0000'0010: pTile->m_townRoadId = 1; break;
            case 0b0000'0011: pTile->m_townRoadId = 1; break;
            case 0b0000'0100: pTile->m_townRoadId = 0; break;
            case 0b0000'1000: pTile->m_townRoadId = 0; break;
            case 0b0000'1100: pTile->m_townRoadId = 0; break;
            case 0b0000'0110: pTile->m_townRoadId = 2; break;
            case 0b0000'1010: pTile->m_townRoadId = 3; break;
            case 0b0000'1001: pTile->m_townRoadId = 4; break;
            case 0b0000'0101: pTile->m_townRoadId = 5; break;
            case 0b0000'1110: pTile->m_townRoadId = 6; break;
            case 0b0000'1101: pTile->m_townRoadId = 7; break;
            case 0b0000'1011: pTile->m_townRoadId = 8; break;
            case 0b0000'0111: pTile->m_townRoadId = 9; break;
            case 0b0000'1111: pTile->m_townRoadId = 10; break;
            default: assert(false && "TerraForm::FindTownRoadId :immpossible town road id \n");
            }
        }
    };
    pMap->ForEachTile(findRoadPosition, pMap);
}

void TerraForm::FindWorldRoadId(WorldMap* pMap)
{
    auto findRoadPosition = [](Tile* pTile, WorldMap* pMap)
    {
        uint8_t position = 0;
        if (pTile->m_isWorldRoad)
        {
            position |= pMap->GetNeighborTile(pTile->m_index, WorldMap::Direction::E)->m_isWorldRoad ? 0b0000'0001 : 0b0000'0000;
            position |= pMap->GetNeighborTile(pTile->m_index, WorldMap::Direction::W)->m_isWorldRoad ? 0b0000'0010 : 0b0000'0000;
            position |= pMap->GetNeighborTile(pTile->m_index, WorldMap::Direction::N)->m_isWorldRoad ? 0b0000'0100 : 0b0000'0000;
            position |= pMap->GetNeighborTile(pTile->m_index, WorldMap::Direction::S)->m_isWorldRoad ? 0b0000'1000 : 0b0000'0000;

            //S N W E
            //
            switch (position)
            {
            case 0b0000'1001: pTile->m_worldRoadId = 0; break;
            case 0b0000'1011: pTile->m_worldRoadId = 1; break;
            case 0b0000'1010: pTile->m_worldRoadId = 2; break;
            case 0b0000'1101: pTile->m_worldRoadId = 3; break;
            case 0b0000'1111: pTile->m_worldRoadId = 4; break;
            case 0b0000'1110: pTile->m_worldRoadId = 5; break;
            case 0b0000'0101: pTile->m_worldRoadId = 6; break;
            case 0b0000'0111: pTile->m_worldRoadId = 7; break;
            case 0b0000'0110: pTile->m_worldRoadId = 8; break;
            default: pTile->m_worldRoadId = 4; break;
            //default: assert(false && "TerraForm::FindTownRoadId :immpossible world road id \n");
            }
        }
    };
    pMap->ForEachTile(findRoadPosition, pMap);
}

void TerraForm::CompressNoise(NoiseType noiseType, WorldMap* pMap, float midPoint)
{
    auto func = [](Tile* pTile, NoiseType inType, float mid)
    {
        switch (inType)
        {
        case TerraForm::NoiseType::Tree:
            pTile->m_treeNoise > mid ? pTile->m_treeNoise = 1.f : pTile->m_treeNoise = BAD_F;
            break;
        case TerraForm::NoiseType::Grass:
            pTile->m_grassNoise > mid ? pTile->m_grassNoise = 1.f : pTile->m_grassNoise = BAD_F;
            
            break;
        default: assert(false && "wrong noise type");
            break;
        }
    };

    pMap->ForEachTile(func, noiseType, midPoint);
}

void TerraForm::SortTreeAndGrass(WorldMap* pMap)
{
    for (int i = 0; i < pMap->GetTileCount(); ++i)
    {
        Tile* pTile = pMap->GetTile(i);

        if (TypePicker::GetTreeTypeFromBiome(pTile->m_biomeId) == TreeType::None)
        {
            pTile->m_treeNoise = BAD_F;
        }

        if (TypePicker::GetGrassTypeFromBiome(pTile->m_biomeId) == GrassType::None)
        {
            pTile->m_grassNoise = BAD_F;
        }

        if (pTile->m_treeNoise > 0)
        {
            //plant tree here, and close off nearby tiles
            auto collection = pMap->GetTilesInRange(pTile->m_index, 1, false);
            for (auto id : collection)
            {
                Tile* pNearbyTile = pMap->GetTile(id);
                pNearbyTile->m_treeNoise = BAD_F;
                pNearbyTile->m_grassNoise = BAD_F;
            }
        }
    }
}

void TerraForm::PickBiome(WorldMap* pMap, const char* pBiomeData)
{
    lua_State* pLua = luaL_newstate();
    luaL_openlibs(pLua);
    if (E2::CheckLua(pLua, (int)(luaL_dofile(pLua, pBiomeData))))
    {
        for (int i = 0; i < pMap->GetTileCount(); ++i)
        {
            auto* pTile = pMap->GetTile(i);
            //lua pops the function (and the args)after it's called so I have to recall it 
            lua_getglobal(pLua, "GetBiomeData");
            if (lua_isfunction(pLua, -1))
            {
                lua_pushnumber(pLua, pTile->m_heightNoise);   /* push 1st argument */
                lua_pushnumber(pLua, pTile->m_temperatureNoise);   /* push 2nd argument */

                // 2 argument, 4 return values
                if (E2::CheckLua(pLua, lua_pcall(pLua, 2, 4, 0)))
                {
                    /* retrieve result */
                    pTile->m_biomeId = (int)lua_tonumber(pLua, -4);
                    assert((pTile->m_biomeId > 0) && "biomeId must exist");
                    pTile->m_biomeColor.r = (uint8_t)lua_tonumber(pLua, -3);
                    pTile->m_biomeColor.g = (uint8_t)lua_tonumber(pLua, -2);
                    pTile->m_biomeColor.b = (uint8_t)lua_tonumber(pLua, -1);
                    lua_pop(pLua, 4);  /* pop returned value */
                }
            }
        }
    }
    lua_close(pLua);
}

std::vector<int> TerraForm::FindTownCenter(WorldMap* pMap, int density, int heightLow, int heightHigh, float tempLow, float tempHigh)
{
    auto tileIsGoodToStartTown = [](Tile* pTile, int heightLow, int heightHigh, float tempLow, float tempHigh) -> bool
    {
        if (pTile)
        {
            if (pTile->m_height > heightLow && pTile->m_height < heightHigh)
            {
                if (pTile->m_temperatureNoise > tempLow && pTile->m_temperatureNoise < tempHigh)
                {
                    return true;
                }
            }
        }
        return false;
    };

    //divide the map in to smaller blocks, each block spawns 0~1 town
    int blockSize = density; //blockSize * blockSize tiles

    int startX = 0;
    int startY = 0;
    int endX = BAD;
    int endY = BAD;
    int maxX = pMap->GetColumnCount();
    int maxY = pMap->GetRowCount();

    endX = (startX + blockSize) < maxX ? (startX + blockSize) : maxX;
    endY = (startY + blockSize) < maxY ? (startY + blockSize) : maxY;

    std::vector<int> townCenters;

    while (startY < maxY)
    {
        std::vector<int> goodTileID;
        for (int y = startY; y < endY; ++y)
        {
            for (int x = startX; x < endX; ++x)
            {
                auto* pTile = pMap->GetTile(x, y);
                if (tileIsGoodToStartTown(pTile, heightLow, heightHigh, tempLow, tempHigh))
                {
                    goodTileID.push_back(pTile->m_index);
                }
            }
        }
        //randomly chose a good tile
        int choice = E2::Rand::PickRandomFrom(goodTileID);
        if (choice >= 0)
        {
            townCenters.push_back(choice);
            pMap->GetTile(choice)->m_isTown = true;
        }

        startX = endX;
        if (startX >= maxX)
        {
            //we reached the end of current row
            startX = 0;
            startY += blockSize;
        }
        endX = (startX + blockSize) < maxX ? (startX + blockSize) : maxX;
        endY = (startY + blockSize) < maxY ? (startY + blockSize) : maxY;
        //std::cout << "Moving on to next block -> "<< i << "\n";
    }
    return townCenters;
}

std::vector<int> TerraForm::BuildTown(WorldMap* pMap, int townSize, float blocks)
{
    //expand town
    std::vector<std::vector<int>> towns;
    for (int id : pMap->GetTownCenters())
    {
        auto newTown = pMap->GetTilesInRange(id, townSize / 2, true);
        for (int newId : newTown)
        {
            Tile* pTile = pMap->GetTile(newId);
            pTile->m_isTown = true;
        }
        towns.emplace_back(newTown);
    }

    auto tileIsGoodToBuildHouse = [pMap](Tile* pTile) -> bool
    {
        int startX = pTile->m_x - 2;
        int startY = pTile->m_y - 2;
        int endX = pTile->m_x;
        int endY = pTile->m_y;
        for (int y = startY; y <= endY; ++y)
        {
            for (int x = startX; x <= endX; ++x)
            {
                Tile* pCurrentTile = pMap->GetTile(x, y);
                if (pCurrentTile)
                {
                    if (pTile->m_height > 6 && pCurrentTile->m_height == pTile->m_height)
                    {
                        if (pCurrentTile->m_isOccupiedBy == BAD)
                        {
                            continue;
                        }
                    }
                }
                return false;
            }
        }
        return true;
    };

    auto buildHouse = [pMap](Tile* pInTile) -> int
    {
        Tile* pDoorTile = nullptr;
        Tile* pSpriteAnchorTile = nullptr;
        int startX = -1;
        int startY = -1;
        int endX = -1;
        int endY = -1;
        Tile::Stuff houseType = Tile::Stuff::Nothing;

        if (CoinFlip())
        {
            //south-facing house
            //o o o
            //o ^ x
            //- - - <- in tile
            //The sprite will render at x
            //- is empty space
            startX = pInTile->m_x - 2;
            startY = pInTile->m_y - 2;
            endX = pInTile->m_x;
            endY = pInTile->m_y - 1;
            pDoorTile = pMap->GetTile(endX - 1, endY);
            pSpriteAnchorTile = pMap->GetTile(endX, endY);
            houseType = Tile::Stuff::SouthHouse;
        }
        else
        {
            //east-facing house
            //o o -
            //o ^ -
            //o x - <- in tile
            //The sprite will render at x
            //- is empty space
            startX = pInTile->m_x - 2;
            startY = pInTile->m_y - 2;
            endX = pInTile->m_x - 1;
            endY = pInTile->m_y;
            pDoorTile = pMap->GetTile(endX, endY - 1);
            pSpriteAnchorTile = pMap->GetTile(endX, endY);
            houseType = Tile::Stuff::EastHouse;
        }
        if (pDoorTile && pSpriteAnchorTile)
        {
            int houseTextureId = 0;
            for (int y = startY; y <= pInTile->m_y; ++y)
            {
                for (int x = startX; x <= pInTile->m_x; ++x)
                {
                    Tile* pCurrentTile = pMap->GetTile(x, y);
                    if (x <= endX && y <= endY)
                    {
                        pCurrentTile->m_stuff = houseType;
                        pCurrentTile->m_houseTextureId = houseTextureId;
                        ++houseTextureId;
                    }
                    pCurrentTile->m_isOccupiedBy = pSpriteAnchorTile->m_index;
                    pCurrentTile->m_grassNoise = BAD_F;
                    pCurrentTile->m_treeNoise = BAD_F;
                }
            }
            pDoorTile->m_isDoor = true;
            return pDoorTile->m_index;
        }
        else
        {
            //error
            return -1;
        }
    };

    std::vector<int> doorTileIDs;
    for (auto& town : towns)
    {
        int blockSize = (int)((float)townSize / blocks); //blockSize * blockSize tiles

        int startX = pMap->GetTile(town[0])->m_x;
        int startY = pMap->GetTile(town[0])->m_y;
        int endX = BAD;
        int endY = BAD;
        int maxX = BAD;
        int maxY = BAD;

        //find boundries 
        for (int id : town)
        {
            if (pMap->GetTile(id)->m_x > maxX)
            {
                maxX = pMap->GetTile(id)->m_x;
            }

            if (pMap->GetTile(id)->m_y > maxY)
            {
                maxY = pMap->GetTile(id)->m_y;
            }
        }
        endX = (startX + blockSize) < maxX ? (startX + blockSize) : maxX;
        endY = (startY + blockSize) < maxY ? (startY + blockSize) : maxY;

        //partition
        while (startY < maxY)
        {
            for (int y = startY; y < endY; ++y)
            {
                for (int x = startX; x < endX; ++x)
                {
                    auto* pTile = pMap->GetTile(x, y);
                    if (tileIsGoodToBuildHouse(pTile))
                    {
                        if (CoinFlip())
                        {
                            int doorTileID = buildHouse(pTile);
                            assert(doorTileID != -1 && "door id error \n");
                            doorTileIDs.push_back(doorTileID);
                        }
                        //build only one house per partition
                        goto Next;
                    }
                }
            }

        Next:

            startX = endX;
            if (startX >= maxX)
            {
                //we reached the end of current row
                startX = pMap->GetTile(town[0])->m_x;
                startY += blockSize;
            }

            endX = (startX + blockSize) < maxX ? (startX + blockSize) : maxX;
            endY = (startY + blockSize) < maxY ? (startY + blockSize) : maxY;
            //std::cout << "Moving on to next block -> "<< i << "\n";
        }
    }
    return doorTileIDs;
}

void TerraForm::BuildRoadsBetweenHouses(WorldMap* pMap, int searchRadius)
{
    auto& doors = pMap->GetHouseDoors();

    std::unordered_map<int, std::vector<int>> doorConnectionList;

    /* find nearby doors */
    auto findNearbyDoors = [pMap, searchRadius](int startDoorId) -> std::vector<int>
    {
        auto nearbyTilesID = pMap->GetTilesInRadius(startDoorId, searchRadius, false); //don't include itself
        std::vector<int> doorIds;

        for (int tileId : nearbyTilesID)
        {
            if (pMap->GetTile(tileId)->m_isDoor)
            {
                doorIds.push_back(tileId);
            }
        }
        return doorIds;
    };

    /* check if these 2 doors are already connected, return true if haven't */
    auto checkDoorConnection = [&doorConnectionList](int fromDoorId, int toDoorId)->bool
    {
        auto neighbors = doorConnectionList.find(fromDoorId);
        if (neighbors == doorConnectionList.end())
        {
            return true;
        }
        else
        {
            auto door = std::find(neighbors->second.begin(), neighbors->second.end(), toDoorId);
            if (door == neighbors->second.end())
            {
                return true;
            }
        }
        return false;
    };

    /* path finding with A* */
    auto connect2Doors = [&doorConnectionList,pMap](int fromDoorId, int toDoorId)->bool //it can fail
    {
        struct PathNode
        {
            int m_tileId = BAD;
            int m_comeFrom = BAD;
            float m_fScore = kMaxFloat;
            float m_gScore = kMaxFloat;
            WorldMap::Direction m_comeFromDirection = WorldMap::Direction::Unknown;
        };

        //helper: this array records which nodes are currently in the openSet. It's an exact copy. 
        //It's difficult to directly access the container inside the priority queue, so we make another one
        std::unordered_map<int,PathNode> nodeIdInTheQueue;  

        //helper: this array records which nodes are done for.
        std::vector<int> closeSet;

        /* priority evaluation */
        auto compFunc = [](PathNode left, PathNode right)
        {
            return left.m_fScore > right.m_fScore;
        };

        //this is the priority queue 
        std::priority_queue<PathNode, std::vector<PathNode>, decltype(compFunc)> openSet(compFunc);
        
        /* helper: see if certain value is in the container */
        auto findId = [](std::vector<int>& container, int value)->bool
        {
            auto itr = std::find(container.begin(), container.end(), value);
            if (itr == container.end())
            {
                return false;
            }
            else
            {
                return true;
            }
        };

        /* G score = (basic score between nodes) evaluation */
        auto getGScore = [pMap](int neighborId, int start, int end, int scoreMagnitude)-> float
        {
            Tile* pNeighborTile = pMap->GetTile(neighborId);

            if (neighborId == end)
            {
                return -10000;
            }
            else if (pNeighborTile->m_isTownRoad)
            {
                return -10.f;
            }
            else if (pNeighborTile->m_stuff != Tile::Stuff::Nothing)
            {
                return 10000.f;     //TODO: cannot use max float here, it will overflow. need a big number.
            }
            else if (pNeighborTile->m_isOccupiedBy != BAD)
            {
                return 300.f;
            }
            else
            {
                auto a = pNeighborTile->m_heightNoise - pMap->GetTile(start)->m_heightNoise;
                auto b = pNeighborTile->m_heightNoise - pMap->GetTile(end)->m_heightNoise;
                return (std::fabs(a) + std::fabs(b)) * (float)scoreMagnitude;
            }
        };

        /* heuristic score evaluation */
        auto heuristic = [pMap](PathNode& current, int neighborId, int end, WorldMap::Direction fromDir) -> float
        {
            auto deltaX = pMap->GetTile(neighborId)->m_x - pMap->GetTile(end)->m_x;
            auto deltaY = pMap->GetTile(neighborId)->m_y - pMap->GetTile(end)->m_y;
            float distance = (float)(deltaX * deltaX + deltaY * deltaY);
            float  multitude = 1.f;
            if (current.m_comeFromDirection != fromDir)
            {
                multitude = 5.f;
            }

            return distance * multitude;
        };

        auto checkNeighbor = [pMap, &openSet, &closeSet, &nodeIdInTheQueue, &findId, &getGScore, &heuristic](PathNode current, int fromId, int toId, WorldMap::Direction dir) ->bool
        {
            auto* pNeighborTile = pMap->GetNeighborTile(current.m_tileId,dir);
            if (!pNeighborTile)
                return false;

            if (findId(closeSet, pNeighborTile->m_index))
                return false;

            float gNew = 0.f;
            float hNew = 0.f;
            float fNew = 0.f;

            gNew = current.m_gScore + getGScore(pNeighborTile->m_index, fromId, toId, 200);
            hNew = heuristic(current,pNeighborTile->m_index, toId, dir);
            fNew = gNew + hNew;

            auto itr = nodeIdInTheQueue.find(pNeighborTile->m_index);
            //if this neighbor is currently in the queue, check if this is a better route
            if (itr != nodeIdInTheQueue.end())      //TODO: Ugly branch
            {
                if (itr->second.m_fScore > fNew)    // greater?
                {
                    PathNode NeighborNode;
                    NeighborNode.m_tileId = pNeighborTile->m_index;
                    NeighborNode.m_comeFrom = current.m_tileId;
                    NeighborNode.m_fScore = fNew;
                    NeighborNode.m_gScore = gNew;
                    NeighborNode.m_comeFromDirection = dir;

                    itr->second = NeighborNode;
                    openSet.push(NeighborNode);     //TODO: shouldn't it update current value instead of pushing again??
                }
            }
            else
            {
                PathNode NeighborNode;
                NeighborNode.m_tileId = pNeighborTile->m_index;
                NeighborNode.m_comeFrom = current.m_tileId;
                NeighborNode.m_fScore = fNew;
                NeighborNode.m_gScore = gNew;
                NeighborNode.m_comeFromDirection = dir;

                openSet.push(NeighborNode);
                nodeIdInTheQueue[NeighborNode.m_tileId] = NeighborNode;
            }

            return true;
        };
        
        

        PathNode startNode;
        /**/
        auto prepareStartNodeDir = [pMap](PathNode& startNode, int startIndex)
        {
            auto stuff = pMap->GetTile(startIndex)->m_stuff;
            startNode.m_tileId = startIndex;
            startNode.m_gScore = 0;
            switch (stuff)
            {
            case Tile::Stuff::EastHouse: startNode.m_comeFromDirection = WorldMap::Direction::E; break;
            case Tile::Stuff::SouthHouse: startNode.m_comeFromDirection = WorldMap::Direction::S; break;
            default: assert(false && "BuildRoadBetweenHouse: prepareStartNodeDir\n"); break;
            }
        };
        prepareStartNodeDir(startNode, fromDoorId);
        openSet.push(startNode);
        while (!(openSet.empty() || openSet.top().m_tileId == toDoorId))
        {
            PathNode currentNode = openSet.top();
            closeSet.emplace_back(currentNode.m_tileId);
            openSet.pop();

            checkNeighbor(currentNode, fromDoorId, toDoorId, WorldMap::Direction::E);
            checkNeighbor(currentNode, fromDoorId, toDoorId, WorldMap::Direction::W);
            checkNeighbor(currentNode, fromDoorId, toDoorId, WorldMap::Direction::N);
            checkNeighbor(currentNode, fromDoorId, toDoorId, WorldMap::Direction::S);
        }

        if (openSet.empty())
        {
            return false;
        }
        else
        {
            //reconstruct path
            int index = openSet.top().m_tileId;

            while (index != BAD)
            {
                Tile* pTile = pMap->GetTile(index);
                if (pTile->m_isDoor)
                {
                    index = nodeIdInTheQueue[index].m_comeFrom;
                    continue;
                }
                else
                {
                    pTile->m_isTownRoad = true;
                    index = nodeIdInTheQueue[index].m_comeFrom;
                }
            }
        }
        return true;
    };

    for (int currentDoorId : doors)
    {
        //find nearby doors around this door
        auto nearbyDoorIds = findNearbyDoors(currentDoorId);
        if (nearbyDoorIds.size() == 0)
        {
            continue; //proceed to next door if this one has no neighbor in radius
        }

        //connect 2 doors
        for (int neighborDoorId : nearbyDoorIds)
        {
            //if they haven't connected
            if (checkDoorConnection(currentDoorId,neighborDoorId))
            {
                //connect them, put them on the list
                if (connect2Doors(currentDoorId, neighborDoorId))
                {
                    doorConnectionList[currentDoorId].push_back(neighborDoorId);
                    doorConnectionList[neighborDoorId].push_back(currentDoorId);
                }
            }
        }
    }
}

bool TerraForm::Connect2Doors(WorldMap* pMap, int fromId, int toId, int distanceLimit)
{
    struct PathNode
    {
        float m_gScore = kMaxFloat;
        float m_fScore = kMaxFloat;
        int m_comeFrom = BAD;
        WorldMap::Direction m_comeFromDirection = WorldMap::Direction::Unknown;
    };

    //helper: this array records which nodes are currently in the openSet.
    std::unordered_map<int, PathNode> nodeInTheQueue;

    //helper: this array records which nodes are done for.
    std::vector<int> closeSet;

    /* priority evaluation */
    auto comp = [&nodeInTheQueue](int left, int right)
    {
        return nodeInTheQueue[left].m_fScore > nodeInTheQueue[right].m_fScore;
    };

    //this is the priority queue 
    std::priority_queue<int, std::vector<int>, decltype(comp)> openSet(comp);

    /* G score = (basic score between nodes) evaluation */
    auto getGScore = [pMap,&nodeInTheQueue](int currentId , int neighborId, int start, int end, WorldMap::Direction fromDir)-> float
    {
        Tile* pSelf = pMap->GetTile(currentId);
        Tile* pNeighborTile = pMap->GetTile(neighborId);

        float base = 50.f;
        float heightPanelty = 0;
        float directionChangePanelty = 0;

        if (pNeighborTile->m_isTownRoad)
        {
            base = 0;
        }
        if (pNeighborTile->m_height != pSelf->m_height)
        {
            heightPanelty = 50.f;
        }
        if (nodeInTheQueue[currentId].m_comeFromDirection != fromDir)
        {
            directionChangePanelty = 50.f;
        }
        if (pNeighborTile->m_index == end)
        {
            return -10000.f;
        }

        return base + heightPanelty + directionChangePanelty;
    };

    /* heuristic score evaluation */
    auto heuristic = [pMap,&nodeInTheQueue](int currentId, int neighborId, int end, WorldMap::Direction fromDir) -> float
    {
        if (pMap->GetTile(neighborId)->m_isWorldRoad)
        {
            return 0;
        }

        float distanceFromThisToEnd2 = (float)pMap->Distance2(currentId, end);
        float distanceFromNeighborToEnd2 = (float)pMap->Distance2(neighborId, end);
        float directionPanelty = distanceFromNeighborToEnd2 < distanceFromThisToEnd2 ? 0 : 50.f;

        float returnValue = distanceFromNeighborToEnd2 > directionPanelty ? distanceFromNeighborToEnd2 : directionPanelty;

        return returnValue;
    };

    auto checkNeighbor = [pMap, &nodeInTheQueue, &openSet, &closeSet, &getGScore, &heuristic](int currentId, int fromId, int toId, WorldMap::Direction dir) ->bool
    {
        auto* pNeighborTile = pMap->GetNeighborTile(currentId, dir);
        if (!pNeighborTile)
            return false;
        bool tileIsClosed = std::find(closeSet.begin(), closeSet.end(), pNeighborTile->m_index) != closeSet.end();
        if (tileIsClosed)
            return false;

        bool tileIsOutOfHeight = pNeighborTile->m_height < 6 || pNeighborTile->m_height > 12;
        bool tileIsBlocked = pNeighborTile->m_stuff != Tile::Stuff::Nothing; 
        if (tileIsOutOfHeight || tileIsBlocked)
        {
            //bad tile, put in the close set
            closeSet.push_back(pNeighborTile->m_index);
            return false;
        }

        float gNew = 0.f;
        float hNew = 0.f;
        float fNew = 0.f;

        gNew = nodeInTheQueue[currentId].m_gScore + getGScore(currentId, pNeighborTile->m_index, fromId, toId, dir);
        hNew = heuristic(currentId, pNeighborTile->m_index, toId, dir);
        fNew = gNew + hNew;

        auto itr = nodeInTheQueue.find(pNeighborTile->m_index);
        //if this neighbor is currently in the queue, check if this is a better route
        if (itr != nodeInTheQueue.end())      //TODO: Ugly branch
        {
            if (itr->second.m_fScore > fNew)    // greater?
            {
                itr->second.m_gScore = gNew;
                itr->second.m_fScore = fNew;
                itr->second.m_comeFrom = currentId;
                itr->second.m_comeFromDirection = dir;
            }
        }
        else
        {
            //new node
            nodeInTheQueue[pNeighborTile->m_index].m_comeFrom = currentId;
            nodeInTheQueue[pNeighborTile->m_index].m_gScore = gNew;
            nodeInTheQueue[pNeighborTile->m_index].m_fScore = fNew;
            nodeInTheQueue[pNeighborTile->m_index].m_comeFromDirection = dir;

            openSet.push(pNeighborTile->m_index);
        }

        return true;
    };



    PathNode startNode;
    /**/
    auto prepareStartNodeDir = [pMap,&nodeInTheQueue](int startIndex)
    {
        nodeInTheQueue[startIndex].m_gScore = 0;
        auto stuff = pMap->GetTile(startIndex)->m_stuff;
        switch (stuff)
        {
        case Tile::Stuff::EastHouse: nodeInTheQueue[startIndex].m_comeFromDirection = WorldMap::Direction::E; break;
        case Tile::Stuff::SouthHouse: nodeInTheQueue[startIndex].m_comeFromDirection = WorldMap::Direction::S; break;
        default: assert(false && "BuildRoadBetweenHouse: prepareStartNodeDir\n"); break;
        }
    };
    prepareStartNodeDir(fromId);
    openSet.push(fromId);
    while (!openSet.empty())
    {
        int currentNodeId = openSet.top();
        if (currentNodeId == toId)
            break;

        closeSet.emplace_back(currentNodeId);
        openSet.pop();

        checkNeighbor(currentNodeId, fromId, toId, WorldMap::Direction::E);
        checkNeighbor(currentNodeId, fromId, toId, WorldMap::Direction::W);
        checkNeighbor(currentNodeId, fromId, toId, WorldMap::Direction::N);
        checkNeighbor(currentNodeId, fromId, toId, WorldMap::Direction::S);
    }

    if (openSet.empty())
    {
        return false;
    }
    else
    {
        //reconstruct path
        int index = openSet.top();

        while (index != BAD)
        {
            index = nodeInTheQueue[index].m_comeFrom;
            Tile* pTile = pMap->GetTile(index);
            if (!pTile->m_isTownRoad)
            {
                pTile->m_isTownRoad = true;
            }
        }
    }
    return true;
}

void TerraForm::BuildRoadsBetweenTowns(WorldMap* pMap, int searchRadius)
{
    auto& towns = pMap->GetTownCenters();

    std::unordered_map<int, std::vector<int>> townConnectionList;

    /* check if these 2 towns are already connected, return true if haven't */
    auto checkTownConnection = [&townConnectionList](int fromDoorId, int toDoorId)->bool
    {
        auto neighbors = townConnectionList.find(fromDoorId);
        if (neighbors == townConnectionList.end())
        {
            return true;
        }
        else
        {
            auto town = std::find(neighbors->second.begin(), neighbors->second.end(), toDoorId);
            if (town == neighbors->second.end())
            {
                return true;
            }
        }
        return false;
    };
    std::cout << "Towns = " << towns.size() << "\n";

    for (int currentTwonId : towns)
    {
        //find nearby
        std::vector<int> nearbyTownIds;

        for (int id : towns)
        {
            if (id != currentTwonId)
            {
                if (pMap->Distance2(currentTwonId, id) < searchRadius * searchRadius)
                {
                    nearbyTownIds.push_back(id);
                }
            }
        }

        if (nearbyTownIds.size() == 0)
        {
            continue; //proceed to next door if this one has no neighbor in radius
        }

        //connect 2 doors
        for (int neighborTownId : nearbyTownIds)
        {
            //if they haven't connected
            if (checkTownConnection(currentTwonId, neighborTownId))
            {
                //connect them, put them on the list
                if (Connect2Towns(pMap, currentTwonId, neighborTownId, searchRadius))
                {
                    townConnectionList[currentTwonId].push_back(neighborTownId);
                    townConnectionList[neighborTownId].push_back(currentTwonId);

#if DRAW_SEARCH_TOWN
                    GetEngine().CleanRenderer();
#endif
                    std::cout << "Connected: " << currentTwonId << " & " << neighborTownId << '\n';
                }
            }
        }
    }

}

bool TerraForm::Connect2Towns(WorldMap* pMap, int fromId, int toId, int distanceLimit)
{
    struct PathNode
    {
        float m_gScore = kMaxFloat;
        float m_fScore = kMaxFloat;
        int m_comeFrom = BAD;
        WorldMap::Direction m_comeFromDirection = WorldMap::Direction::Unknown;
    };

    //helper: this is the container of the openSet. 
    //It's difficult to directly access the container inside the priority queue, so we make one
    std::unordered_map<int, PathNode> nodeInTheQueue;

    //helper: this array records which nodes are done for.
    std::vector<int> closeSet;

    /* priority evaluation */
    auto comp = [&nodeInTheQueue](int left, int right)
    {
        if (!nodeInTheQueue.empty())
        {
            return nodeInTheQueue[left].m_fScore > nodeInTheQueue[right].m_fScore;
        }
        return false;
    };
    //this is the priority queue 
    std::priority_queue<int, std::vector<int>, decltype(comp)> openSet(comp);

    /* G score = (basic score between nodes) evaluation */
    auto getGScore = [pMap, &nodeInTheQueue](int currentId, int neighborId, int endId, WorldMap::Direction fromDir)-> float
    {
        Tile* pSelf = pMap->GetTile(currentId);
        Tile* pNeighborTile = pMap->GetTile(neighborId);

        float base = 50.f;
        float heightPenalty = 0;
        float directionChangePanelty = 0;
        float edgePenalty = 0;
        if (pNeighborTile->m_isTown || pNeighborTile->m_isWorldRoad)
        {
            base = 0;
        }
        if (pNeighborTile->m_height != pSelf->m_height)
        {
            heightPenalty = 50.f;
        }
        if (nodeInTheQueue[currentId].m_comeFromDirection != fromDir)
        {
            directionChangePanelty = 50.f;
        }
        if (pNeighborTile->m_blockId != 4)
        {
            edgePenalty = 50;
        }
        if (pNeighborTile->m_index == endId)
        {
            return -10000.f;
        }

        return base + heightPenalty + directionChangePanelty + edgePenalty;

    };

    /* heuristic score evaluation */
    auto heuristic = [pMap, &nodeInTheQueue](int currentId, int neighborId, int end, WorldMap::Direction fromDir) -> float
    {
        if (pMap->GetTile(neighborId)->m_isWorldRoad)
        {
            return 0;
        }

        float distanceFromThisToEnd2 = (float)pMap->Distance2(currentId, end);
        float distanceFromNeighborToEnd2 = (float)pMap->Distance2(neighborId, end);
        float directionPanelty = distanceFromNeighborToEnd2 < distanceFromThisToEnd2 ? 0 : 50.f;

        float returnValue = distanceFromNeighborToEnd2 > directionPanelty ? distanceFromNeighborToEnd2 : directionPanelty;

        
        return returnValue;
    };

    auto checkNeighbor = [pMap, &nodeInTheQueue, &closeSet, &openSet, &getGScore, &heuristic](int currentId, int fromId, int toId, int distanceLimit, WorldMap::Direction dir) ->bool
    {
        auto* pNeighborTile = pMap->GetNeighborTile(currentId, dir);
        if (!pNeighborTile)
            return false;
        bool tileIsClosed = std::find(closeSet.begin(), closeSet.end(), pNeighborTile->m_index) != closeSet.end();
        if (tileIsClosed)
            return false;

        bool tileIsOutOfHeight = pNeighborTile->m_height < 6 || pNeighborTile->m_height > 11;
        bool tileIsOccupied = pNeighborTile->m_isOccupiedBy != BAD;
        bool tileTooFar = pMap->Distance2(pNeighborTile->m_index, fromId) > (distanceLimit * distanceLimit);
        if (tileIsOutOfHeight || tileIsOccupied || tileTooFar)
        {
            //bad tile, put in the close set
            closeSet.push_back(pNeighborTile->m_index);
            return false;
        }

        float gNew = 0.f;
        float hNew = 0.f;
        float fNew = 0.f;

        gNew = nodeInTheQueue[currentId].m_gScore + getGScore(currentId, pNeighborTile->m_index, toId, dir);
        hNew = heuristic(currentId, pNeighborTile->m_index, toId, dir);
        fNew = gNew + hNew;

        auto itr = nodeInTheQueue.find(pNeighborTile->m_index);
        //if this neighbor is currently in the queue, check if this is a better route
        if (itr != nodeInTheQueue.end())      //TODO: Ugly branch
        {
            if (itr->second.m_fScore > fNew)    // greater?
            {
                itr->second.m_gScore = gNew;
                itr->second.m_fScore = fNew;
                itr->second.m_comeFrom = currentId;
                itr->second.m_comeFromDirection = dir;
            }
        }
        else
        {
            //new node
            nodeInTheQueue[pNeighborTile->m_index].m_comeFrom = currentId;
            nodeInTheQueue[pNeighborTile->m_index].m_gScore = gNew;
            nodeInTheQueue[pNeighborTile->m_index].m_fScore = fNew;
            nodeInTheQueue[pNeighborTile->m_index].m_comeFromDirection = dir;

            openSet.push(pNeighborTile->m_index);
        }
        return true;
    };

    nodeInTheQueue[fromId].m_gScore = 0;

    openSet.push(fromId);
    while (!openSet.empty())
    {
        int currentNodeId = openSet.top();
        if (currentNodeId == toId)
        {
            break;
        }
        closeSet.push_back(currentNodeId);
        openSet.pop();

        checkNeighbor(currentNodeId, fromId, toId, distanceLimit, WorldMap::Direction::E);
        checkNeighbor(currentNodeId, fromId, toId, distanceLimit, WorldMap::Direction::W);
        checkNeighbor(currentNodeId, fromId, toId, distanceLimit, WorldMap::Direction::N);
        checkNeighbor(currentNodeId, fromId, toId, distanceLimit, WorldMap::Direction::S);

#if DRAW_SEARCH_TOWN
        auto* pTile = pMap->GetTile(currentNodeId);
        DrawPoint(pTile->m_x, pTile->m_y, E2::RedColor::kPink);
        GetEngine().RenderNow();
#endif
    }

    if (openSet.empty())
    {
        return false;
    }
    else
    {
        //reconstruct path
        int current = openSet.top();
        while (current != BAD)
        {
            Tile* pCurrentTile = pMap->GetTile(current);
            if (!pCurrentTile->m_isTown)
            {
                pCurrentTile->m_isWorldRoad = true;
            }
            current = nodeInTheQueue[current].m_comeFrom;
        }
    }
    return true;
}

