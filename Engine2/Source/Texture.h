#pragma once
#include "Vector2.h"
#include "Macro.h"
namespace E2
{
    enum class TextureType
    {
        BMP,
        JPG,
        PNG,
        Text,
        Unknown,
    };
    struct Texture
    {
    public:
        TextureType type;
        Vector2 dimension;
        _Texture* pTexture;

    public:
        Texture();
        ~Texture();
    };
}
