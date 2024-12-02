#pragma once
#include "Vector2.h"
#include "EventListener.h"
#include "Component.h"

#include <typeinfo>
#include <string>

namespace E2
{
    class GameObject : public EventListener
    {
    private:
        int m_id = -1;
        std::unordered_map<uint32_t, Component*>m_components;
        std::vector<std::string> m_tags;
    public:
        GameObject();
        ~GameObject();

        void SetId(int id) { m_id = id; }
        int GetId() const { return m_id; };

        void AddTag(std::string str) { m_tags.emplace_back(str); }
        const std::vector<std::string>& GetTags()const { return m_tags; }

        void Update(float deltaTime);
        void Draw();

        void AddComponent(Component* pComponent);
        void AddComponent(const char* pComponentType);
        Component* GetComponent(const char* pName);

        //void SetPosition(const Vector2f& pos) {}
        //void SetRotation(float rot) {}
        //E2::Vector2f GetPosition() const { return E2::Vector2f(); }
        //Component* GetTransform() { return nullptr; }
    };
}