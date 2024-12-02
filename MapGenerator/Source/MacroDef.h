#pragma once
#define BAD -1
#define BAD_F -1.f

//shouldn't be here :
constexpr int kTemperatureInputRange = 5;
constexpr const char* kBiomeData = "Script/Biome.lua";

constexpr float kMinCityHeight = 0.5f;
constexpr float kMaxCityHeight = 0.7f;

constexpr int kDefaultInputRange = 10;
constexpr int kMaxInputRange = 15;
constexpr int kDefaultOctaves = 4;
constexpr int kMaxOctaves = 5;
constexpr float kDefaultPersistence = 0.6f;
constexpr float kMaxPersistence = 1.f;

constexpr int kMaxWorldSplits = 10;
constexpr int kMinSplitSize = 20;
constexpr int kSmoothDistanceFromSplit = 4;

constexpr int kMaxHeight = 15;
constexpr int kSeaHeight = 6;

constexpr int kTreeDensity = 100;
constexpr float kTreeThreashold = 0.55f;

constexpr int kMapSizeX = 300;  //initial size, before expansion
constexpr int kMapSizeY = 200;

constexpr int kViewRange = 80;
constexpr int kMapExpandMagnitude = 3;

constexpr int kTownDensity = 100;
constexpr int kTownSize = 20;