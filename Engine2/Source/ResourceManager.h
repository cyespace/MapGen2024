#pragma once
#include "pch.h"
#include "Texture.h"
#include "Font.h"
#include <filesystem>
#include <cstddef>

namespace E2
{
    using Path = std::filesystem::path;
    using Resource = std::vector<std::byte>;
    class ResourceManager
    {
    private:
        std::unordered_map<size_t, Resource> m_resources;
        std::unordered_map<size_t, Texture> m_textures;
        std::unordered_map<size_t, std::string> m_itemList;
    public:
        ResourceManager() = default;
        ~ResourceManager();

        bool Load(Path path);
        std::byte* GetResource(size_t id);
        size_t GetResourceSize(size_t id);
        Texture GetTexture(size_t id);
        void SaveTexture(size_t id, Texture texture);
        TextureType GetTextureType(size_t id);
        void DestroyAll();
    };
}