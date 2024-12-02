#include "pch.h"
#include "Engine.h"
#include "ResourceManager.h"
#include "HashString.h"
#include <fstream>

E2::ResourceManager::~ResourceManager()
{
    //Destroy all Textures
    DestroyAll();
}

bool E2::ResourceManager::Load(Path path)
{
    auto hash = HashString(path.generic_string());
    if (m_itemList.find(hash) != m_itemList.end())
    {
        //we already loaded this.
        return false;
    }
    else
    {
        m_itemList.emplace(hash, path.generic_string());
    }

    std::ifstream file;
    file.open(path,std::ios::binary);
    if (!file.is_open())
    {
        return false;
    }

    auto size = std::filesystem::file_size(path);

    std::vector<std::byte> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    m_resources.emplace(hash, std::move(buffer));
    
    file.close();
    return true;
}

std::byte* E2::ResourceManager::GetResource(size_t id)
{
    if (m_resources.find(id) != m_resources.end())
    {
        return m_resources[id].data();
    }
    return nullptr;
}

size_t E2::ResourceManager::GetResourceSize(size_t id)
{
    if (m_resources.find(id) != m_resources.end())
    {
        return m_resources[id].size();
    }
    return 0;
}

E2::Texture E2::ResourceManager::GetTexture(size_t id)
{
    if (m_textures.find(id) != m_textures.end())
    {
        return m_textures[id];
    }
    return {};
}

void E2::ResourceManager::SaveTexture(size_t id, Texture texture)
{
    m_textures.emplace(id, texture);
}

E2::TextureType E2::ResourceManager::GetTextureType(size_t id)
{
    Path p = m_itemList[id];
    std::string extension = p.extension().generic_string();
    if (extension == ".bmp")
    {
        return TextureType::BMP;
    }
    else if(extension == ".png")
    {
        return TextureType::PNG;
    }
    else if (extension == ".jpg")
    {
        return TextureType::JPG;
    }
    else if (extension == ".txt")
    {
        return TextureType::Text;
    }
    return TextureType::Unknown;
}

void E2::ResourceManager::DestroyAll()
{
    if (!m_textures.empty())
    {
        for (auto& [id, texture] : m_textures)
        {
            Engine::Get().DestroyTexture(texture);
        }
        m_textures.clear();
    }
}
