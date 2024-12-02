#pragma once
#include "Engine.h"
#include "Random.h"

inline E2::Engine& GetEngine() { return E2::Engine::Get(); }

inline void DrawRect(const E2::Rect& rect, const E2::Color& color)
{
    E2::Engine::Get().DrawRect(rect, color);
}

inline void DrawRectOutline(const E2::Rect& rect, const E2::Color& color)
{
    E2::Engine::Get().DrawRectOutline(rect, color);
}

inline void DrawPoint(int x, int y, const E2::Color& color)
{
    E2::Engine::Get().DrawPoint(x, y, color);
}

inline void DrawTexture(E2::Texture& texture, E2::Rect* pSrc, E2::Rect* pDest)
{
    E2::Engine::Get().DrawTexture(texture, pSrc, pDest);
}

inline E2::Color RandomColor()
{
    auto r = (uint8_t)E2::Rand::Random(0,255);
    auto g = (uint8_t)E2::Rand::Random(0,255);
    auto b = (uint8_t)E2::Rand::Random(0,255);
    return E2::Color{ r,g,b };
}

inline bool CoinFlip()
{
    return E2::Rand::RandomBool(2);
}