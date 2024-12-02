#pragma once
#include "Component.h"

#include "Vector2.h"
#include "Rect.h"
namespace E2
{
    class GameObject;
    class TransformComponent : public Component
    {
    private:
        Vector2f m_position;
        Vector2f m_dimension;
        float m_rotation;
    public:
        TransformComponent(GameObject* pOwner);
        TransformComponent(GameObject* pOwner, float x, float y, float w, float h, float rotation);
        virtual ~TransformComponent() {}

        void SetPosition(float x, float y) { m_position.x = x; m_position.y = y; }
        void SetPosition(const Vector2f& pos) { m_position = pos; }
        void SetDimension(float w, float h) { m_dimension.x = w; m_dimension.y = h; }
        void SetRotation(float radian) { m_rotation = radian; }
        void SetRotation(Vector2f front);

        Vector2f GetPosition() const { return m_position; }
        Vector2f GetDimension() const { return m_dimension; }
        float GetRotation() const { return m_rotation; }
        Vector2f GetCenter() const
        {
            return Vector2f{ (m_position.x + m_dimension.x/2), (m_position.y + m_dimension.y/2)};
        }
        Rect GetRect() const { return Rect{ (int)m_position.x,(int)m_position.y,(int)m_dimension.x,(int)m_dimension.y}; }
        virtual uint32_t GetTypeId()const override;
    };

}