#pragma once

enum class BlockType
{
    Dirt,
    Ice,
    Rock,
    Sand,
    Water,
    Undefined,
};

enum class SurfaceType
{
    Grass,
    Snow,
    Forest,
    Water,
    Ice,
    None,
    Debug,
    Undefined,
};

enum class TreeType
{
    Cold,
    Green,
    Yellow,
    Rain,
    HalfSnow,
    Dead,
    None,
};

enum class GrassType
{
    Cold,
    Green,
    Rain,
    HalfSnow,
    Yellow,
    Tall,
    None,
};

enum class HouseType
{
    SouthHouse,
    EastHouse,
};

namespace TypePicker
{
    inline BlockType GetBlockTypeFromBiome(int id)
    {
        BlockType blockType = BlockType::Undefined;
        switch (id)
        {
        case 0: blockType = BlockType::Dirt; break;
        case 1: blockType = BlockType::Dirt; break;
        case 2: blockType = BlockType::Dirt; break;
        case 3: blockType = BlockType::Dirt; break;
        case 4: blockType = BlockType::Dirt; break;
        case 5: blockType = BlockType::Sand; break;
        case 6: blockType = BlockType::Sand; break;
        case 7: blockType = BlockType::Dirt; break;
        case 8: blockType = BlockType::Dirt; break;
        case 9: blockType = BlockType::Dirt; break;
        case 10: blockType = BlockType::Dirt; break;
        case 11: blockType = BlockType::Dirt; break;
        case 12: blockType = BlockType::Dirt; break;
        case 13: blockType = BlockType::Dirt; break;
        case 14: blockType = BlockType::Rock; break;
        case 15: blockType = BlockType::Rock; break;
        case 16: blockType = BlockType::Rock; break;
        case 17: blockType = BlockType::Sand; break;
        case 18: blockType = BlockType::Rock; break;
        case 19: blockType = BlockType::Rock; break;
        case 20: blockType = BlockType::Rock; break;
        case 21: blockType = BlockType::Ice; break;
        case 22: blockType = BlockType::Ice; break;
        default: blockType = BlockType::Undefined; break;
        }
        return blockType;
    }

    inline SurfaceType GetSurfaceTypeFromBiome(int id)
    {
        SurfaceType surfaceType = SurfaceType::Undefined;
        switch (id)
        {
        case 1: surfaceType = SurfaceType::Snow; break;
        case 2: surfaceType = SurfaceType::Ice; break;
        case 3: surfaceType = SurfaceType::Water; break;
        case 4: surfaceType = SurfaceType::Water; break;
        case 5: surfaceType = SurfaceType::Water; break;
        case 6: surfaceType = SurfaceType::None; break;
        case 7: surfaceType = SurfaceType::Grass; break;
        case 8: surfaceType = SurfaceType::Grass; break;
        case 9: surfaceType = SurfaceType::Grass; break;
        case 10: surfaceType = SurfaceType::Snow; break;
        case 11: surfaceType = SurfaceType::Forest; break;
        case 12: surfaceType = SurfaceType::Forest; break;
        case 13: surfaceType = SurfaceType::Snow; break;
        case 14: surfaceType = SurfaceType::Grass; break;
        case 15: surfaceType = SurfaceType::Grass; break;
        case 16: surfaceType = SurfaceType::None; break;
        case 17: surfaceType = SurfaceType::None; break;
        case 18: surfaceType = SurfaceType::None; break;
        case 19: surfaceType = SurfaceType::None; break;
        case 20: surfaceType = SurfaceType::Grass; break;
        case 21: surfaceType = SurfaceType::None; break;
        case 22: surfaceType = SurfaceType::Snow; break;
        default: surfaceType = SurfaceType::Undefined; break;
        }
        return surfaceType;
    }

    //TODO: 
    inline TreeType GetTreeTypeFromBiome(int id)
    {
        TreeType treeType = TreeType::None;
        switch (id)
        {
        case 7:
        case 8:
        case 9:  treeType = TreeType::Green; break;
        case 10: treeType = TreeType::Cold; break;
        case 11: treeType = TreeType::Rain; break;
        case 12: treeType = TreeType::Green; break;
        case 13: treeType = TreeType::HalfSnow; break;
        case 14: treeType = TreeType::Cold; break;
        case 15: treeType = TreeType::Yellow; break;
        case 16: treeType = TreeType::Dead; break;
        case 22: treeType = TreeType::HalfSnow; break;
        default: break;
        }

        return treeType;
    }

    inline GrassType GetGrassTypeFromBiome(int id)
    {
        GrassType grassType = GrassType::None;
        switch (id)
        {
        case 7:  grassType = GrassType::Green; break;
        case 8:  grassType = GrassType::Tall; break;
        case 9:  grassType = GrassType::Green; break;
        case 10: grassType = GrassType::Cold; break;
        case 11: grassType = GrassType::Rain; break;
        case 12: grassType = GrassType::Rain; break;
        case 13: grassType = GrassType::HalfSnow; break;
        case 14: grassType = GrassType::Cold; break;
        case 15: grassType = GrassType::Yellow; break;
        case 22: grassType = GrassType::HalfSnow; break;
        default: break;
        }
        return grassType;
    }
}