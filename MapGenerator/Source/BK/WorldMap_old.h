#pragma once

#include <Color.h>

#include <vector>
#include <utility>
#include <unordered_map>
struct Tile;
class Grid;
struct Region;

class WorldMap
{
public:
    using SplitLine = std::pair<Tile*, Tile*>;
    using PathNodeId = int;
    struct PathNode
    {
        float m_gScore = std::numeric_limits<float>::max();
        float m_fScore = std::numeric_limits<float>::max();
        PathNodeId m_cameFrom = -1;
    };
private:
    // W and H as in tile count
    int m_mapW;
    int m_mapH;
    int m_tileSize;
    int m_currentInputRange;
    int m_currentOctaves;
    float m_currentPersistence;
    Grid* m_pGrid = nullptr; 
    Grid* m_pWorldData = nullptr;
    Region* m_pRootRegion;

    std::vector<PathNode> m_pathNodes;
    std::vector<Tile*> m_cities;
    std::vector<std::vector<int>> m_cityAdjacencyList;

    std::vector<Tile*> m_roads;

    std::vector<int> m_survey;
public:
    WorldMap(int width,int height, int tileSize);
    ~WorldMap();

    //Height Noise
    void GenerateBasicHeightNoise();
    void AdjustRegionHeightNoise();
    void ReShapeHeightNoise();
    void SmoothNoiseAroundSplitLines();
    void DrawHeightNoise();
    void DrawWorld();

    //Temperature Noise
    void GenerateTemperatureNoise(int equatorLine, int complexity);
    void DrawTemperature();

    void GenerateBiome(const char* pBiomeData);

    //Adjust Regions
    void SplitRegion(size_t times);
    void DrawSplitLines();
    
    //Cities and Roads
    void FoundCity(float minHeight,float maxHeight);
    int GetCityCount(Region* pNode);
    void LinkCities();
    void BuildRoads();
    void FindPath(PathNodeId start, PathNodeId end);
    void ExpandCityTiles(Tile* pCenter, int range);

    //Misc
    void ChangeInputRange(int delta);
    void ChangePersistence(float delta);
    void ChangeOctave(int delta);

    void ResetPathNode();
    void Reset();
    
    Tile* GetTile(int x, int y); 
    void DoSurvey();
    void PrintInfo();

    //Rework needed
    void Expand(int magnitude);
    Grid* GetWorldData() { return m_pWorldData; }

private:
    float RegionEmptiness(int regionTileCount, int slope);
};
