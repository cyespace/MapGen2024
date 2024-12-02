#pragma once
#include "Building.h"
#include "Tree.h"
#include "Grass.h"
#include "LocalMapElementDef.h"

#include <Texture.h>
#include <Event.h>
#include <EventListener.h>

#include <vector>
#include <unordered_map>
#include <map>
namespace E2
{
    struct Color;
}

enum class TilePosition
{
    TopLeft,
    Top,
    TopRight,
    MidLeft,
    Mid,
    MidRight,
    BottomLeft,
    Bottom,
    BottomRight,
    Unknown,
};

enum class TileSet
{
    GrassyTerrain,
    SnowyTerrain,
    DryTerrain,
    RockyTerrain,
    UnderWater0,
    UnderWater1,
    UnderWater2,
    UnderWater3,

    Beach,
    CoastLine,
    ShallowWater,
    DeepWater,
    ThinIce,
    ThickIce,

    Grass,
    Dirt,
    Forest,
    Ice,
    Sand,

    Road,
    TownRoad,

    Undefined,
};


struct TileNode
{
    int m_tileId = -1;
    bool m_isTested = false; // 
    float m_localHeight = -1.f;
    bool m_isWalkable = true;
    bool m_occupied = false;
    bool m_canOpen = false;
    bool m_isRoad = false;
    bool m_isGrass = false;
    int m_position = -1; //??
    TilePosition m_pos = TilePosition::Unknown;
    //bool isUnderneath = false;
    int m_grassId = -1;
    int m_treeId = -1;
};

struct TileTexture
{
    TileSet tileSetId = TileSet::GrassyTerrain;
    int textureId = -1;
};

class GameObject;
struct Region;
class LocalMap : public E2::EventListener
{
    using SplitLine = std::pair<Tile*, Tile*>;
    using City = std::pair<Tile*, Tile*>;
    using TileId = int;

    using PathNodeId = int;
    struct PathNode
    {
        float m_gScore = std::numeric_limits<float>::max();
        float m_fScore = std::numeric_limits<float>::max();
        PathNodeId m_cameFrom = -1;
    };
private:
    Grid* m_pGrid;
    std::vector<City> m_cities;
    std::vector<Region*> m_rootRegions;

    //area building
    std::vector<TileNode> m_tileNodes;
    std::vector<std::vector<int>> m_areas;
    std::vector<E2::Color> m_colors;

    std::unordered_map<TileId, BuildingType> m_buildableTiles;
    std::vector<TileId> m_doorSteps;
    std::vector<TileId> m_roads;
    std::vector<PathNode> m_pathNodes;

    //tileSet
    std::vector<TileTexture> m_tileTextures;
    std::unordered_map<TileSet,E2::Texture> m_tileSets;
    std::unordered_map<BuildingType,E2::Texture> m_buildingSets;
    std::unordered_map<TreeType,E2::Texture> m_treeSets;
    std::unordered_map<GrassType,E2::Texture> m_grassSets;

    //MapObject
    std::vector<StaticMapElement*> m_treesAndBuildings;
    std::vector<StaticMapElement*> m_grass;

    //Isometric 
    //tileSet
    std::unordered_map<BlockType, std::vector<E2::Texture>> m_blockTextures;
    std::unordered_map<SurfaceType, std::vector<E2::Texture>> m_surfaceTextures;
    std::unordered_map<GrassType, E2::Texture> m_grassTextures;
    std::unordered_map<TreeType, E2::Texture> m_treeTextures;


    
public:
    LocalMap();
    ~LocalMap();

    Tile* GetTile(int x, int y) { return m_pGrid->GetTile(x, y); }
    Tile* GetTile(int id) { return m_pGrid->GetTile(id); }
    bool IsWalkAble(int tileId);

    void Reset(int maxX, int maxY, int tileSize);
    void ReFreshTileNodesForSearching();
    void ReFreshPathNodesForPathing();
    void CopyTileInfo(int index, Tile* pInTile);
    void CopyTileInfo(int x, int y, Tile* pInTile);
    void Terrace(int level);
    std::vector<float> GetPerlinNoiseMap(float interval, float zoomInLevel);

    //Adjust Regions
    void SplitRegion(Region* pRoot);
    void GenerateBuildableRegions();
    void ConnectBuildings();// deprecated

    void DrawHeight();
    void DrawHeightTerrace(int level);

    // Apply TileSet
    void LoadArea(float level);
    void BuildArea(int nodeId);
    void DrawArea();
    void LoadTerrainTexture();
    void LoadTilesetTexture();
    void BuildTown();
    void BuildTownRoad();
    void ConnectAreas();

    void PopulateTrees();
    void PopulateGrass();

    //AI 
    std::vector<TileId> GetSpawnAbleTiles();
    std::vector<TileId> GetReachableTiles(E2::Vector2f origin, float radius);
    E2::Vector2f GetRandomNearPositionInNav(E2::Vector2f origin, float radius);
    std::vector<E2::Vector2f> BuildPath(E2::Vector2f from, E2::Vector2f to);
    //Misc
    void DrawAreaColor();
    virtual void OnNotify(E2::Event evt) override;

    //Isometric 
    void LoadIsometricTexture();
    void DrawTerrainIsometric();
    void GetTilePositionTest();
    BlockType GetBlockTypeFromBiome(int id);
    SurfaceType GetSurfaceTypeFromBiome(int id);
private:
    void FindCities();
    void SpawnBuildings();
    bool IsTileInCity(Tile* pTile);
    void SetTileSetId();

    void FindPath(PathNodeId start, PathNodeId end);
};