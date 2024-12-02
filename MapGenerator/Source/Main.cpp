#include "MapGenerator.h"
#include "Isometric.h"
#include <GlobalFunctions.h>

#ifdef _DEBUG
//#include <vld.h>
#endif // _DEBUG

#ifdef TEST
#undef TEST
#endif
#define TEST 0

#ifdef main
#undef main
#endif // main

int main()
{
#if TEST
    GetEngine().Run(IsometricTest::Get());
#else
    GetEngine().Run(MapGenerator::Get());
#endif
    return 0;
}
