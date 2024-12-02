#pragma once
#include <cstdint>

// Xorshift* 
// https://en.wikipedia.org/wiki/Xorshift
namespace E2
{
    namespace Rand
    {
        void SetSeed(uint64_t seed);
        uint64_t GetSeed();

        uint64_t Random();
        uint64_t Random(uint64_t min, uint64_t max);
        //int Random(int min, int max);
        bool RandomBool(int base);

        float RandomF(float min, float max);

        int PickRandomFrom(std::vector<int>& container);
    }
}