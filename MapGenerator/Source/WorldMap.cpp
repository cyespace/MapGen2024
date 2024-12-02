#include "WorldMap.h"
#include <assert.h>
#include <cmath>

WorldMap::~WorldMap()
{
    m_worldData.clear();
    m_townCenters.clear();
    m_houseDoors.clear();
}

//TODO: The actual x,y coordinates of a tile might not be useful to be calculated here.
void WorldMap::Init(int coloumn, int row)
{
    m_column = coloumn;
    m_row = row;

    m_worldData = std::vector<Tile>(coloumn * row);    //world map is created here.

    int index = 0;
    for (int y = 0; y < row; ++y)
    {
        for (int x = 0; x < coloumn; ++x)
        {
            auto& currentTile = m_worldData[index];    // initiate each tile.
            currentTile.m_index = index;
            currentTile.m_x = x;
            currentTile.m_y = y;
            ++index;
        }
    }
}

int WorldMap::GetTileIndex(int x, int y) const
{
    if (x >= m_column || y >= m_row || x < 0 || y < 0)
    {
        return BAD;
    }
    return x + y * m_column;
}

E2::Vector2 WorldMap::GetTileXY(int index) const
{
    if (index < m_worldData.size() && index >= 0)
    {
        return { m_worldData[index].m_x,  m_worldData[index].m_y };
    }
    return E2::Vector2{ BAD, BAD };
}

Tile* WorldMap::GetTile(int index)
{
    if (index < m_worldData.size() && index >= 0)
    {
        return &m_worldData[index];
    }
    else
    {
        return nullptr;
    }
}

Tile* WorldMap::GetTile(int x, int y)
{
    return GetTile(GetTileIndex(x,y));
}

int WorldMap::GetNeighborIndex(int centerTileIndex, Direction dir) const
{
    auto pos = GetTileXY(centerTileIndex);
    assert(pos.x != BAD); //the center tile must exist.

    switch (dir)
    {
    case Direction::N: return GetTileIndex(pos.x, pos.y - 1);
    case Direction::S: return GetTileIndex(pos.x, pos.y + 1);
    case Direction::E: return GetTileIndex(pos.x + 1, pos.y);
    case Direction::W: return GetTileIndex(pos.x - 1, pos.y);;
    case Direction::NE: return GetTileIndex(pos.x + 1, pos.y - 1);;
    case Direction::NW: return GetTileIndex(pos.x - 1, pos.y - 1);;
    case Direction::SE: return GetTileIndex(pos.x + 1, pos.y + 1);;
    case Direction::SW: return GetTileIndex(pos.x - 1, pos.y + 1);;
    default: break;
    }
    return BAD;
}

Tile* WorldMap::GetNeighborTile(int centerTileIndex, Direction dir)
{
    return GetTile(GetNeighborIndex(centerTileIndex,dir));
}

int WorldMap::Distance2(int tileA, int tileB)
{
    Tile* pA = GetTile(tileA);
    Tile* pB = GetTile(tileB);
    if (pA && pB)
    {
        return Distance2(pA, pB);
    }
    else
    {
        return BAD;
    }
}

const int WorldMap::Distance2(Tile* pA, Tile* pB)
{
    return (pA->m_x - pB->m_x)* (pA->m_x - pB->m_x) + (pA->m_y - pB->m_y)* (pA->m_y - pB->m_y);
}

//Return the index of the tile in this position, or -1 if none-exist
//int WorldMap::FindTile(E2::Vector2f position)
//{
//    int xCoord = std::lroundf(position.x);
//    int yCoord = std::lroundf(position.y);
//    int x = xCoord / m_tileLength;
//    int y = yCoord / m_tileLength;
//
//
//    if (x <0 || x > m_column || y <0 || y > m_row)
//    {
//        return BAD;
//    }
//    else
//    {
//        return GetTileIndex(x,y);
//    }
//}

bool WorldMap::CopyTile(int from, int to)
{
    return CopyTile(GetTile(from),GetTile(to));
}

bool WorldMap::CopyTile(Tile* pFrom, Tile* pTo)
{
    if (pFrom && pTo)
    {
        *pTo = *pFrom;

        return true;
    }
    else
    {
        return false;
    }
}

std::vector<int> WorldMap::GetTilesInRadius(int centerTileId, int radius, bool includeSelf)
{
    assert(GetTile(centerTileId)&&"tile out of map!");

    std::vector<int> collection;

    int startX = GetTile(centerTileId)->m_x;
    int startY = GetTile(centerTileId)->m_y;

    for (int y = startY - radius; y <= startY + radius; ++y)
    {
        for (int x = startX - radius; x <= startX + radius; ++x)
        {
            int distance2 = (x - startX) * (x - startX) + (y - startY) * (y - startY);
            if (distance2 < radius * radius)
            {
                Tile* pTile = GetTile(x, y);
                if (pTile)
                {
                    if (pTile->m_index == centerTileId && !includeSelf)
                    {
                        continue;
                    }
                    collection.push_back(pTile->m_index);
                }
            }
        }
    }

    return collection;
}

std::vector<int> WorldMap::GetTilesInRange(int centerTileId, int unitRange, bool includeSelf)
{
    assert(GetTile(centerTileId) && "tile out of map!");

    std::vector<int> collection;

    int startX = GetTile(centerTileId)->m_x;
    int startY = GetTile(centerTileId)->m_y;

    for (int y = startY - unitRange; y <= startY + unitRange; ++y)
    {
        for (int x = startX - unitRange; x <= startX + unitRange; ++x)
        {
            Tile* pTile = GetTile(x, y);
            if (pTile)
            {
                if (pTile->m_index == centerTileId && !includeSelf)
                {
                    continue;
                }
                collection.push_back(pTile->m_index);
            }
        }
    }

    return collection;
}

void WorldMap::Expand(int magnitude)
{
    int oldColumn = m_column;
    int oldRow = m_row;
    int newColumn = oldColumn * magnitude;
    int newRow = oldRow * magnitude;
    std::vector<Tile> newWorldData(newColumn * newRow);
    int newIndex = 0;
    for (int y = 0; y < newRow; ++y)
    {
        for (int x = 0; x < newColumn; ++x)
        {
            int oldX = x / magnitude;
            int oldY = y / magnitude;
            newWorldData[newIndex] = *(GetTile(oldX,oldY));

            //new index!
            newWorldData[newIndex].m_index = newIndex;
            newWorldData[newIndex].m_x = x;
            newWorldData[newIndex].m_y = y;

            ++newIndex;
        }
    }
    m_column = newColumn;
    m_row = newRow;
    m_worldData.clear();
    m_worldData = newWorldData;
    newWorldData.clear();
}
