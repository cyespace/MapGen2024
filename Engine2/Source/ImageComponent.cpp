#include "pch.h"
#include "ImageComponent.h"
#include "TransformComponent.h"
#include "GameObject.h"
#include "Engine.h"

E2::ImageComponent::ImageComponent(GameObject* pOwner)
    : Component{pOwner}
{
    m_pTransform = nullptr;
}

E2::ImageComponent::ImageComponent(GameObject* pOwner, Texture texture)
    : ImageComponent(pOwner)
{
    AddImage(texture);
}

E2::ImageComponent::ImageComponent(GameObject* pOwner, const char* pTextureDir)
    : ImageComponent(pOwner)
{
    AddImage(pTextureDir);
}

void E2::ImageComponent::AddImage(Texture pTexture)
{
    m_texture = pTexture;
}

void E2::ImageComponent::AddImage(const char* pTextureDir)
{
    m_texture = Engine::Get().CreateTexture(pTextureDir);
}

void E2::ImageComponent::Draw()
{
    auto rect = m_pTransform->GetRect();
    Engine::Get().DrawTexture(m_texture,nullptr,&(rect));
}
