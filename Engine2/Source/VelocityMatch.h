#pragma once
#include "Steering.h"
namespace E2
{
    class VelocityMatch : public Steering
    {
    protected:
        float m_maxAcceleration = 0;
        float m_timeToTarget = 0;
    public:
        VelocityMatch(float maxAcceleration, float timeToTarget);
        SteeringOutput GetSteering(GameObject* pA, GameObject* pB) override;
    };

    class Arrive : public Steering
    {
    protected:

        float m_maxSpeed = 0;
        float m_maxAcceleration = 0;

        float m_slowRadius = 0;
        float m_arriveRadius = 0;

        float m_timeToTarget = 0;

        Vector2f m_positionOffset = {};
    public:
        Arrive(float maxSpeed
            , float maxAcceleration
            , float slowRadius
            , float arriveRadius
            , float timeToTarget);
        SteeringOutput GetSteering(GameObject* pA, GameObject* pB) override;
        SteeringOutput GetSteering(GameObject* pA, const E2::Vector2f pos);
    };

}