#include "pch.h"
#include "UIImage.h"
#include "Engine.h"

E2::UIImage::UIImage(Texture texture, Rect src, Rect dest,UIDesignCoord x, UIDesignCoord y
    , UIDesignCoord w, UIDesignCoord h, UIDesignCoord x0, UIDesignCoord y0)
    : m_texture{ texture }
    , m_isSliced{ false }
{
    if (src.w == 0 || src.h == 0)
    {
        src.w = m_texture.dimension.x;
        src.h = m_texture.dimension.y;
    }
    if (dest.w == 0 || dest.h == 0)
    {
        dest.w = m_texture.dimension.x;
        dest.h = m_texture.dimension.y;
    }

    m_sourceRects.emplace_back(src);
    SetPosition(x,y);
    SetDimension(w,h);
    SetOrigin(x0,y0);
    Reposition(dest.x, dest.y, dest.w, dest.h);
    m_destRects.emplace_back(m_realRegion);
}

E2::UIImage::~UIImage()
{
    m_sourceRects.clear();
    m_destRects.clear();
}

void E2::UIImage::Draw()
{
    if (m_isVisable)
    {
        if (!m_isSliced)
        {
            E2::Engine::Get().DrawTexture(m_texture, &m_sourceRects[0], &m_destRects[0]);
        }
        else
        {
            for (int i = 1; i < m_sourceRects.size(); ++i)
            {
                E2::Engine::Get().DrawTexture(m_texture, &m_sourceRects[i], &m_destRects[i]);
            }
        }
    }
}

void E2::UIImage::SetImage(Texture texture, E2::Rect newSource)
{
    assert(!m_isSliced && "E2::UIImage::SetImage: Cannot change source img of a sliced UI\n");
    m_texture = texture;
    if (newSource.h == 0 || newSource.w == 0)
    {
        m_sourceRects[0].w = texture.dimension.x;
        m_sourceRects[0].h = texture.dimension.y;
    }
    else
    {
        m_sourceRects[0] = newSource;
    }
}

void E2::UIImage::SetSlice(UIDesignCoord left, UIDesignCoord right, UIDesignCoord top, UIDesignCoord down)
{
    m_isSliced = true;
#pragma warning(disable:4244)
    int srcLeftW = left.Resolve(m_sourceRects[0].w);
    int srcMidW = right.Resolve(m_sourceRects[0].w) - left.Resolve(m_sourceRects[0].w);
    int srcRightW = m_sourceRects[0].w - right.Resolve(m_sourceRects[0].w);
    int srcTopH = top.Resolve(m_sourceRects[0].h);
    int srcMidH = down.Resolve(m_sourceRects[0].h) - top.Resolve(m_sourceRects[0].h);
    int srcDownH = m_sourceRects[0].h - down.Resolve(m_sourceRects[0].h);

    Rect srcTopLeft { m_sourceRects[0].x , m_sourceRects[0].y, srcLeftW , srcTopH };
    Rect srcTopMid  { m_sourceRects[0].x + srcLeftW , m_sourceRects[0].y , srcMidW , srcTopH };
    Rect srcTopRight{ m_sourceRects[0].x + srcLeftW + srcMidW , m_sourceRects[0].y , srcRightW , srcTopH };

    Rect srcMidLeft { m_sourceRects[0].x , m_sourceRects[0].y + srcTopH, srcLeftW, srcMidH };
    Rect srcMidMid  { m_sourceRects[0].x + srcLeftW, m_sourceRects[0].y + srcTopH, srcMidW, srcMidH };
    Rect srcMidRight{ m_sourceRects[0].x + srcLeftW + srcMidW,m_sourceRects[0].y + srcTopH,srcRightW,srcMidH };

    Rect srcDownLeft{ m_sourceRects[0].x, m_sourceRects[0].y + srcTopH + srcMidH, srcLeftW,srcDownH };
    Rect srcDownMid { m_sourceRects[0].x + srcLeftW, m_sourceRects[0].y + srcTopH + srcMidH,srcMidW, srcDownH };
    Rect srcDownRight{ m_sourceRects[0].x + srcLeftW + srcMidW,m_sourceRects[0].y + srcTopH + srcMidH,srcRightW,srcDownH };
   
    m_sourceRects.emplace_back(srcTopLeft);
    m_sourceRects.emplace_back(srcTopMid);
    m_sourceRects.emplace_back(srcTopRight);
    m_sourceRects.emplace_back(srcMidLeft);
    m_sourceRects.emplace_back(srcMidMid);
    m_sourceRects.emplace_back(srcMidRight);
    m_sourceRects.emplace_back(srcDownLeft);
    m_sourceRects.emplace_back(srcDownMid);
    m_sourceRects.emplace_back(srcDownRight);

    int destMidW = m_destRects[0].w - srcLeftW - srcRightW;
    int destMidH = m_destRects[0].h - srcTopH - srcDownH;

    Rect destTopLeft{ m_destRects[0].x , m_destRects[0].y , srcLeftW , srcTopH };
    Rect destTopMid{ m_destRects[0].x + srcLeftW ,m_destRects[0].y, destMidW , srcTopH };
    Rect destTopRight{ m_destRects[0].x + srcLeftW + destMidW, m_destRects[0].y, srcRightW, srcTopH };

    Rect destMidLeft{ m_destRects[0].x, m_destRects[0].y + srcTopH, srcLeftW, destMidH };
    Rect destMidMid{ m_destRects[0].x + srcLeftW, m_destRects[0].y + srcTopH,destMidW,destMidH };
    Rect destMidRight{ m_destRects[0].x + srcLeftW + destMidW, m_destRects[0].y + srcTopH, srcRightW,destMidH };

    Rect destDownLeft{ m_destRects[0].x, m_destRects[0].y + srcTopH + destMidH, srcLeftW, srcDownH };
    Rect destDownMid{ m_destRects[0].x + srcLeftW, m_destRects[0].y + srcTopH + destMidH, destMidW, srcDownH };
    Rect destDownRight{ m_destRects[0].x + srcLeftW + destMidW,m_destRects[0].y + srcTopH + destMidH, srcRightW, srcDownH };

    m_destRects.emplace_back(destTopLeft);
    m_destRects.emplace_back(destTopMid);
    m_destRects.emplace_back(destTopRight);
    m_destRects.emplace_back(destMidLeft);
    m_destRects.emplace_back(destMidMid);
    m_destRects.emplace_back(destMidRight);
    m_destRects.emplace_back(destDownLeft);
    m_destRects.emplace_back(destDownMid);
    m_destRects.emplace_back(destDownRight);

#pragma warning(default:4244)
}
