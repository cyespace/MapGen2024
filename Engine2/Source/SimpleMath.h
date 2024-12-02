#pragma once
#include "Vector2.h"
constexpr float kPi = 3.1415926535f;
constexpr float kMaxFloat = std::numeric_limits<float>::max();
namespace E2
{
    inline Vector2f Lerp(Vector2f min, Vector2f max, float weight)
    {
        return  min*(1 - weight) + max * weight;
    }

    inline float Lerp(float min, float max, float weight)
    {
        return  (1 - weight) * min + weight * max;
    }

    inline float SmoothStep(float x)
    {
        if (x < 0)
            return 0;
        if (x > 1)
            return 1;

        return x * x * (3.f - 2.f * x);
    }

    inline float Absolute(float x)
    {
        if (x > 0)
        {
            return x;
        }
        else if(x < 0)
        {
            return x * -1;
        }
        else
        {
            return x;
        }
    }

    template<typename type>
    type Clamp(type min, type max, type value)
    {
        if (value > max)
        {
            return max;
        }
        if (value < min)
        {
            return min;
        }
        return value;
    }
}