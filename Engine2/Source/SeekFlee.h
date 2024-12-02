#pragma once
#include "Steering.h"

namespace E2
{
    class Seek : public Steering
    {
    protected:
        Vector2f m_thisPos = {};
        Vector2f m_targetPos = {};
        float m_maxAcceleration = 0;
    public:
        Seek(float maxAcceleration);
        SteeringOutput GetSteering(GameObject* pA, GameObject* pB) override;
        SteeringOutput GetSteering(GameObject* pA, const Vector2f pos);
    };

    class Pursue : public Seek
    {
    protected:
        float m_maxPrediction = 0;

    public:
        Pursue(float maxAcceleration, float maxPredictTime);

        SteeringOutput GetSteering(GameObject* pA, GameObject* pB) override;
    };
}
