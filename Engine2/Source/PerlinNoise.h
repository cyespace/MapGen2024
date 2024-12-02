#pragma once
#include "vector2.h"

namespace PerlinNoise
{
    float Perlin(float x, float y, size_t seed);
    float DotGridGradient(int originX, int originY, float x, float y, size_t seed);
    E2::Vector2f UnitGradient();
    E2::Vector2f UnitGradient(int x, int y, size_t seed);
}
