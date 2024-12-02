#include "Region.h"

#include <Random.h>
#include <stack>

constexpr int kInvalid = std::numeric_limits<int>::max();

Region::Region()
    : m_pParent{ nullptr }
    , m_pLeft{ nullptr }
    , m_pRight{ nullptr }
    , m_zone{ {kInvalid,kInvalid},{kInvalid,kInvalid} }
    , m_innerZone{ {kInvalid,kInvalid},{kInvalid,kInvalid} }
    , m_splitLine{ {kInvalid,kInvalid},{kInvalid,kInvalid} }
    , m_depth{ 0 }
{
}

Region::~Region()
{
    delete m_pLeft;
    delete m_pRight;
}

int Region::TileCount()
{
    return (m_zone.second.x - m_zone.first.x + 1) * (m_zone.second.y - m_zone.first.y + 1);
}

void Region::Split(int minSplitSize)
{
    //get the long side 
    int horizontalLength = m_zone.second.x - m_zone.first.x;
    int verticalLength = m_zone.second.y - m_zone.first.y;

    //Split vertically
    if (horizontalLength > verticalLength)
    {
        //the split shall happen closer to the center
        int leftX = m_zone.first.x + minSplitSize;
        int rightX = m_zone.second.x - minSplitSize;
        if (leftX >= rightX)
        {
            return;
        }
        int splitX = (int)E2::Rand::Random(leftX, rightX);
        m_pLeft = new Region();
        m_pRight = new Region();
        m_pLeft->m_pParent = this;
        m_pRight->m_pParent = this;
        m_pLeft->m_depth = m_depth + 1;
        m_pRight->m_depth = m_depth + 1;

        m_pLeft->m_zone.first = m_zone.first;
        m_pLeft->m_zone.second = { splitX - 1, m_zone.second.y };

        m_pRight->m_zone.first = { splitX + 1, m_zone.first.y };
        m_pRight->m_zone.second = m_zone.second;

        RegionTile splitLineStart = { splitX , m_zone.first.y };
        RegionTile splitLineEnd = { splitX , m_zone.second.y };
        m_splitLine = { splitLineStart ,splitLineEnd };
    }
    //Split horizontally
    else
    {
        //the split shall happen closer to the center
        int topY = m_zone.first.y + minSplitSize;
        int bottomY = m_zone.second.y - minSplitSize;
        if (topY >= bottomY)
        {
            return;
        }
        int splitY = (int)E2::Rand::Random(topY, bottomY);

        m_pLeft = new Region();
        m_pRight = new Region();
        m_pLeft->m_pParent = this;
        m_pRight->m_pParent = this;
        m_pLeft->m_depth = m_depth + 1;
        m_pRight->m_depth = m_depth + 1;

        m_pLeft->m_zone.first = m_zone.first;
        m_pLeft->m_zone.second = { m_zone.second.x, splitY - 1};

        m_pRight->m_zone.first = { m_zone.first.x, splitY + 1};
        m_pRight->m_zone.second = m_zone.second;

        RegionTile splitLineStart = {m_zone.first.x, splitY};
        RegionTile splitLineEnd = {m_zone.second.x, splitY};
        m_splitLine = {splitLineStart,splitLineEnd};
    }
}

std::vector<Region*> Region::FindSplittableRegions(Region* pRoot)
{
    std::vector<Region*> availableNodes;
    std::stack<Region*> helper;
    // do an inorder walk
    Region* pCurrentNode = pRoot;
    while (pCurrentNode || !helper.empty())
    {
        while (pCurrentNode)
        {
            if (!pCurrentNode->m_pLeft && !pCurrentNode->m_pRight)
            {
                availableNodes.push_back(pCurrentNode);
            }

            helper.push(pCurrentNode);
            pCurrentNode = pCurrentNode->m_pLeft;
        }

        pCurrentNode = helper.top();
        helper.pop();

        pCurrentNode = pCurrentNode->m_pRight;
    }
    return availableNodes;
}

void Region::DefineZone(int startX, int startY, int endX, int endY)
{
    m_zone = { {startX,startY}, {endX,endY} };
}
