#pragma once

#include "Component.h"
#include "Texture.h"

namespace E2
{
    class GameObject;
    class TransformComponent;
    class ImageComponent : public Component
    {
    private:
        TransformComponent* m_pTransform = nullptr;
        Texture m_texture;
    public:
        ImageComponent(GameObject* pOwner);
        ImageComponent(GameObject* pOwner, Texture texture);
        ImageComponent(GameObject* pOwner, const char* pTextureDir);
        ~ImageComponent() = default;

        void AddImage(Texture pTexture);
        void AddImage(const char* pTextureDir);

        void virtual Draw() override;
    };
}

