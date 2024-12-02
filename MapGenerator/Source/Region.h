#pragma once
#include <Vector2.h>

#include <vector>
#include <utility>


using RegionTile = E2::Vector2;
using Zone = std::pair<RegionTile, RegionTile>;
class Region
{
private:
    Region* m_pParent;
    Region* m_pLeft;
    Region* m_pRight;
    Zone m_zone;
    Zone m_innerZone;
    Zone m_splitLine;
    int m_depth;

public:
    Region();
    ~Region();

    int TileCount();
    
    void Split(int minSplitSize);
    static std::vector<Region*> FindSplittableRegions(Region* pRoot);

    void DefineZone(int startX, int startY, int endX, int endY);
    Zone GetZone()const { return m_zone; }
    Zone GetSplitLine()const { return m_splitLine; }
    template<typename Func, typename... Targs>
    static void WalkTheTree(Region* pRoot, Func&& function, Targs... inArgs);
};

template<typename Func, typename... Targs>
inline static void Region::WalkTheTree(Region* pRoot, Func&& function, Targs... inArgs)
{
    if (!pRoot)
    {
        return;
    }
    WalkTheTree(pRoot->m_pLeft, function, inArgs...);
    WalkTheTree(pRoot->m_pRight, function, inArgs...);
    function(pRoot, inArgs...);
}
