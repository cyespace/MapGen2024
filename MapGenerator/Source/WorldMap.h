#pragma once

#include "MacroDef.h"

#include <Rect.h>
#include <Color.h>
#include <Vector2.h>

#include <vector>

//the basic structure of the world
struct Tile
{
    enum class Stuff
    {
        Nothing,
        SouthHouse,
        EastHouse,
    };

    int m_index = BAD;    //the index of this tile
    int m_x = BAD;        //the x index 
    int m_y = BAD;        //the y index
    float m_heightNoise = BAD_F;  //
    float m_temperatureNoise = BAD_F;   //
    int m_height = BAD;     //actual map height
    int m_biomeId = BAD;
    int m_citySize = BAD;

    int m_blockId = BAD;   //texture Id of the terrain
    int m_surfaceId = BAD;     //texture Id of the surface on terrain
    int m_townRoadId = BAD;
    int m_worldRoadId = BAD;
    int m_critterId = BAD;
    int m_houseTextureId = BAD;

    float m_treeNoise = BAD_F;
    float m_grassNoise = BAD_F;

    Stuff m_stuff = Stuff::Nothing;
    int m_isOccupiedBy = BAD;

    bool m_isTown = false;
    bool m_isDoor = false;
    bool m_isTownRoad = false;
    bool m_isWorldRoad = false;

    E2::Color m_biomeColor = {};
};

//the grand world map
class WorldMap
{
public:
    enum class Direction
    {
        N,
        S,
        E,
        W,
        NE,
        NW,
        SE,
        SW,
        Unknown,
    };
private:

    int m_column = BAD;   // the number of columns of this map
    int m_row = BAD;  // the number of rows of this map
    int m_maxHeight = BAD;
    std::vector<Tile> m_worldData = {};

    std::vector<int> m_townCenters = {};
    std::vector<int> m_houseDoors = {};

public:
    WorldMap() = default;
    ~WorldMap();

    void Init(int coloumn, int row);

    int GetTileIndex(int x, int y) const;
    E2::Vector2 GetTileXY(int index) const;
    Tile* GetTile(int index);
    Tile* GetTile(int x, int y);

    Tile* GetLastTile() { return &m_worldData.back(); }
    size_t GetTileCount() const { return m_worldData.size(); }
    int GetColumnCount() const { return m_column; }
    int GetRowCount() const { return m_row; }

    void SetMaxHeight(int height) { m_maxHeight = height; }
    int GetMaxHeight() const { return m_maxHeight; }

    int GetNeighborIndex(int centerTileIndex, Direction dir) const;
    Tile* GetNeighborTile(int centerTileIndex, Direction dir);

    const std::vector<int>& GetTownCenters() const { return m_townCenters; }
    void SetTownCenters(std::vector<int>& collection) { m_townCenters = collection; }

    const std::vector<int>& GetHouseDoors() const { return m_houseDoors; }
    void SetHouseDoors(std::vector<int>& collection) { m_houseDoors = collection; }

    //distance as in grid distance squared, not pixel distance
    int Distance2(int tileA, int tileB);
    static const int Distance2 (Tile* pA, Tile* pB);    // private?

    //int FindTile(E2::Vector2f position);

    bool CopyTile(int from, int to);
    static bool CopyTile(Tile* pFrom, Tile* pTo);
    
    //should be faster without safty check
    template<typename Func, typename... Targs>
    void ForEachTile(Func&& function, Targs... inArgs);

    //not helpful at all
    Tile* operator[](int index) { return GetTile(index); }

    //radius as in units of tile, not pixels, circle
    std::vector<int> GetTilesInRadius(int centerTileId, int radius, bool includeSelf); 
    //squaire
    std::vector<int> GetTilesInRange(int centerTileId, int unitRange, bool includeSelf);

    //eg if magnitude = 3, 1 tile will become 3*3 = 9 tiles afterwards
    void Expand(int magnitude = 3);
};


template<typename Func, typename... Targs>
inline void WorldMap::ForEachTile(Func&& function, Targs... inArgs)
{
    for (auto& tile : m_worldData)
    {
        function(&tile, inArgs...);
    }
}
