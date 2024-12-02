#include "pch.h"
#include "Random.h"
#include "SimpleMath.h"
#include <assert.h>

static uint64_t s_seed = 0;

void E2::Rand::SetSeed(uint64_t seed)
{
    s_seed = seed;
}

uint64_t E2::Rand::GetSeed()
{
    return s_seed;
}

uint64_t E2::Rand::Random()
{
    uint64_t x = s_seed;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    s_seed = x;

    return x * 0x2545F4914F6CDD1DULL;
}

uint64_t E2::Rand::Random(uint64_t min, uint64_t max)
{
    assert(max >= min);
    if (max == min)
    {
        return min;
    }
    auto range = max - min +1;
    auto randomNum = Random();

    return randomNum % range + min;
}

bool E2::Rand::RandomBool(int base)
{
    assert(base >= 1);
    auto randomNum = Random();
    auto resault = randomNum % base;

    return resault == 0;
}


float E2::Rand::RandomF(float min, float max)
{
    assert(max > min);
    float weight = (float)Random(0, 1000) / 1000.f;
    return E2::Lerp(min, max, weight);
}

int E2::Rand::PickRandomFrom(std::vector<int>& container)
{
    if (container.size() < 1)
    {
        return -1;
    }

    auto randomID = Random(0, container.size() - 1);
    return container[randomID];
}
