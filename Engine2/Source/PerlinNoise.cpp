#include "pch.h"
#include "PerlinNoise.h"
#include "SquirrelNoise.h"
#include "Random.h"
#include "SimpleMath.h"

#include <cmath>
#include <iostream>

float PerlinNoise::Perlin(float x, float y , size_t seed)
{

    int x0 = (int)x;
    int x1 = x0 + 1;
    int y0 = (int)y;
    int y1 = y0 + 1;
    float dX = E2::SmoothStep(x - (float)x0);
    float dY = E2::SmoothStep(y - (float)y0);

    float dotProductTopLeft = DotGridGradient(x0,y0,x,y, seed);
    float dotProductTopRight = DotGridGradient(x1,y0,x,y, seed);
    float dotProductBottomLeft = DotGridGradient(x0,y1,x,y, seed);
    float dotProductBottomRight = DotGridGradient(x1,y1,x,y, seed);

    float xWeight1 = E2::Lerp(dotProductTopLeft, dotProductTopRight, dX);
    float xWeight2 = E2::Lerp(dotProductBottomLeft, dotProductBottomRight, dX);
    float noise = E2::Lerp(xWeight1, xWeight2, dY);

    return noise;
}

float PerlinNoise::DotGridGradient(int originX, int originY, float x, float y , size_t seed)
{
    E2::Vector2f randomUnitVector = UnitGradient(originX, originY, seed);
    E2::Vector2f ourVector{x - (float)originX, y - (float)originY};

    return E2::Vector2f::Dot(randomUnitVector,ourVector);
}

// TODO: 'true' random doesn't work for gradient vector
E2::Vector2f PerlinNoise::UnitGradient()
{
    float radian = E2::Rand::RandomF(0,kPi*2.f);
    return E2::Vector2f{std::sin(radian),std::cos(radian)};
}

E2::Vector2f PerlinNoise::UnitGradient(int x, int y , size_t seed)
{
    float weight = SquirrelNoise::GetNormalized2DNoise(x , y, static_cast<unsigned int>(seed));
    float radian = E2::Lerp(0, kPi * 2.f, weight);
    return E2::Vector2f{ std::sin(radian),std::cos(radian) };
}
