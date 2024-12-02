#include "pch.h"
#include "GameObject.h"
#include "Component.h"

#include "Engine.h"
#include "HashString.h"

E2::GameObject::GameObject()
{
}

E2::GameObject::~GameObject()
{
    for (auto& [id,pComponent]: m_components)
    {
        delete pComponent;
        pComponent = nullptr;
    }
    m_components.clear();
}

void E2::GameObject::Update(float deltaTime)
{
    for (auto& [id, pComponent]: m_components)
    {
        pComponent->Update(deltaTime);
    }
}

void E2::GameObject::Draw()
{
    for (auto& [id, pComponent] : m_components)
    {
        pComponent->Draw();
    }
}

void E2::GameObject::AddComponent(Component* pComponent)
{
    m_components[pComponent->GetTypeId()] = (pComponent);
}

E2::Component* E2::GameObject::GetComponent(const char* pName)
{
    auto id = E2::Crc32(pName);

    auto itr = m_components.find((uint32_t)id);
    if (itr != m_components.end())
    {
        return itr->second;
    }
    else
    {
        return nullptr;
    }
}