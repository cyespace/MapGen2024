#include "pch.h"
#include "Texture.h"
#include "Engine.h"

E2::Texture::Texture()
    : type{ TextureType::Unknown }
    , dimension{ 0,0 }
    , pTexture{ nullptr }
{}

E2::Texture::~Texture()
{
}
