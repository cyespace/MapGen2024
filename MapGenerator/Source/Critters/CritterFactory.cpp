#include "CritterFactory.h"
#include "../WorldMap.h"
#include <../Source/Random.h>
#include <../Source/GlobalFunctions.h>
#include <../Source/GameObject.h>

void CritterFactory::SpawnCritters(WorldMap* pMap)
{
    auto spawn = [](Tile* pTile)
    {
        if (pTile->m_grassNoise > 0)
        {
            //maybeSpawn critter
            auto chance = E2::Rand::Random(0, 100);

            if (chance < 3)
            {
                pTile->m_critterId = 3;
            }
            else if (chance < 8)
            {
                pTile->m_critterId = CoinFlip() ? 1 : 2;
            }
        }
    };
    pMap->ForEachTile(spawn);
}
