#include "pch.h"
#include "SquirrelNoise.h"

constexpr unsigned int kPrime1 = 198491317;
constexpr unsigned int kPrime2 = 6543989;

unsigned int SquirrelNoise::Get1DNoise(int x, unsigned int seed)
{
    constexpr unsigned int kBitNoise1 = 0x68e31da4;
    constexpr unsigned int kBitNoise2 = 0xb5297a4d;
    constexpr unsigned int kBitNoise3 = 0x1b56c4e9;

    auto mangledBits = static_cast<unsigned int>(x);
    mangledBits *= kBitNoise1;
    mangledBits += seed;
    mangledBits ^= (mangledBits >> 8);
    mangledBits *= kBitNoise2;
    mangledBits ^= (mangledBits << 8);
    mangledBits *= kBitNoise3;
    mangledBits ^= (mangledBits >> 8);
    return mangledBits;
}

unsigned int SquirrelNoise::Get2DNoise(int x, int y, unsigned int seed)
{
    return Get1DNoise(x + (kPrime1 * y), seed);
}

unsigned int SquirrelNoise::Get3DNoise(int x, int y, int z, unsigned int seed)
{
    return Get1DNoise(x + (kPrime1 * y) + (kPrime2 * z), seed);
}

float SquirrelNoise::GetNormalized1DNoise(int x, unsigned int seed)
{
    auto noise = static_cast<float>(Get1DNoise(x, seed));
    return noise / static_cast<float>(0xffffffff);
}

float SquirrelNoise::GetNormalized2DNoise(int x, int y, unsigned int seed)
{
    auto noise = static_cast<float>(Get2DNoise(x, y, seed));
    return noise / static_cast<float>(0xffffffff);
}

float SquirrelNoise::GetNormalized3DNoise(int x, int y, int z, unsigned int seed)
{
    auto noise = static_cast<float>(Get3DNoise(x, y, z, seed));
    return noise / static_cast<float>(0xffffffff);
}
