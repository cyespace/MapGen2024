#pragma once

namespace SquirrelNoise
{
    unsigned int Get1DNoise(int x, unsigned int seed);
    unsigned int Get2DNoise(int x, int y, unsigned int seed);
    unsigned int Get3DNoise(int x, int y, int z, unsigned int seed);

    float GetNormalized1DNoise(int x, unsigned int seed);
    float GetNormalized2DNoise(int x, int y, unsigned int seed);
    float GetNormalized3DNoise(int x, int y, int z, unsigned int seed);
}
