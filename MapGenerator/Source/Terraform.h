#pragma once

#include <vector>

class WorldMap;
class Region;
namespace TerraForm
{
    enum class NoiseType
    {
        Height,
        Temperature,
        Tree,
        Grass,
    };

    //TODO : Region style
    enum class RegionFormType
    {
        Islands,
        BigContinents,
        etc,
    };

    //all parameter must be positive
    void BasicPerlinNoise(NoiseType noiseType, WorldMap* pMap, int octaves, int inputRange, float persistence);
    void StretchNoise(NoiseType noiseType, WorldMap* pMap, float min = 0, float max = 1.f);
    void FormulateHeight(WorldMap* pMap, int maxHeight);

    void SplitRegion(Region* pRoot, WorldMap* pMap, int numberOfTimes, int minRegionSize);
    void AdjustRegionalHeightNoise(Region* pRoot, WorldMap* pMap);
    float CheckRegionEmptiness(int zoneTileCount, int worldTileCount, int slope);
    void SmoothNoiseCrossRegion(Region* pRoot, WorldMap* pMap, int smoothDistance);

    //equator: e.g. if the map height is 800, equator = 400 will put the warmest temp at the middle of the map
    void AdjustTemperatureNoise(WorldMap* pMap, int equator);
    void SmoothTemperature(WorldMap* pMap);

    //uint8_t bugs out
    void FindTileSetOffset(unsigned char pos, int& xOffset, int& yOffset);
    void FindBlockId(WorldMap* pMap);
    void FindSurfaceId(WorldMap* pMap);
    void FindTownRoadId(WorldMap* pMap);
    void FindWorldRoadId(WorldMap* pMap);

    //TODO: 
    void CompressNoise(NoiseType noiseType, WorldMap* pMap, float midPoint);
    void SortTreeAndGrass(WorldMap* pMap);

    void PickBiome(WorldMap* pMap, const char* pBiomeData);

    //more density = less town
    std::vector<int> FindTownCenter(WorldMap* pMap, int density, int heightLow, int heightHigh, float tempLow, float tempHigh);
    std::vector<int> BuildTown(WorldMap* pMap, int townSize, float blocks);
    void BuildRoadsBetweenHouses(WorldMap* pMap, int searchRadius);
    bool Connect2Doors(WorldMap* pMap, int fromId, int toId, int distanceLimit);
    void BuildRoadsBetweenTowns(WorldMap* pMap, int searchRadius);
    bool Connect2Towns(WorldMap* pMap, int fromId, int toId, int distanceLimit);

}
