#pragma once
#include "Component.h"
#include "Vector2.h"

namespace E2
{
    class GameObject;
    struct SteeringOutput;
    class KinematicComponent : public Component
    {
    private:
        Vector2f m_position = {};
        float m_orientation = 0;

        Vector2f m_velocity = {};
        float m_rotation = 0;

        Vector2f m_linearAcceleration = {};
        float m_angularAcceleration = 0;

        float m_speedLimit = 0;

    public:
        KinematicComponent(GameObject* pOwner);
        ~KinematicComponent() {}
        virtual void Update(float deltaTime) final;
        float NewOrientation(Vector2f velocity);

        void ApplyAcceleration(SteeringOutput steering);
        void ApplyKinematic(SteeringOutput steering);
        
        void SetSpeedLimit(float maxSpeed) { m_speedLimit = maxSpeed; }
        void SetPosition(const Vector2f& pos) { m_position = pos; }
        void SetRotation(float rot) { m_orientation = rot; }
        void Stop();

        Vector2f GetVelocity()const { return m_velocity; }
    };
}