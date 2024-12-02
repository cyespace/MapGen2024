#pragma once

namespace E2
{
    class GameObject;
    class Component
    {
    protected:
        uint32_t m_typeId = 0;
        GameObject* m_pOwner = nullptr;
    public:
        Component(GameObject* pOwner);
        virtual ~Component() = default;
        virtual void Update(float deltaTime){};
        virtual void Draw(){}
        virtual uint32_t GetTypeId()const;
    };
}