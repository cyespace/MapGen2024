#pragma once
#include "Steering.h"

namespace E2
{
    class Align : public Steering
    {
    protected:
        float m_thisOrientation = 0;
        float m_targetOrientation = 0;

        float m_maxRotation = 0;
        float m_maxAngularAcceleration = 0;

        float m_slowRadius = 0;
        float m_arriveRadius = 0;

        float m_timeToArrive = 0;

    public:
        Align(float maxRotation
            , float maxAngularAcceleration
            , float slowRadius
            , float arriveRadius
            , float timeToTarget);
        SteeringOutput GetSteering(GameObject* pA, GameObject* pB) override;

    };

    class Face : public Align
    {
    public:
        Face( float maxRotation
            , float maxAngularAcceleration
            , float slowRadius
            , float arriveRadius
            , float timeToTarget);
        SteeringOutput GetSteering(GameObject* pA, GameObject* pB) override;
    };

    class LookWhereYoureGoing : public Align
    {
    public:
        LookWhereYoureGoing(float maxRotation
            , float maxAngularAcceleration
            , float slowRadius
            , float arriveRadius
            , float timeToTarget);
        SteeringOutput GetSteering(GameObject* pA, GameObject* pB) override;

    };
}