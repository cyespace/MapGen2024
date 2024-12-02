#include "pch.h"
#include "SeekFlee.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "KinematicComponent.h"

E2::Seek::Seek(float maxAcceleration)
    : m_maxAcceleration{ maxAcceleration }
{
}

E2::SteeringOutput E2::Seek::GetSteering(GameObject* pA, GameObject* pB)
{
    SteeringOutput result;

    //direction
    if (pA)
    {
        m_thisPos = pA->GetTransform()->GetCenter();
    }
    if (pB)
    {
        m_targetPos = pB->GetTransform()->GetCenter();
    }
    result.linear = m_targetPos - m_thisPos;
    //velocity is along this direction, at full speed
    result.linear.Normalize();
    result.linear *= m_maxAcceleration;

    //face the direction
    //m_pObject->GetKinematic()->NewOrientation(result.linear);

    return result;
}

//TODO: Dup codes
E2::SteeringOutput E2::Seek::GetSteering(GameObject* pA, const Vector2f pos)
{
    SteeringOutput result;

    //direction
    if (pA)
    {
        m_thisPos = pA->GetTransform()->GetCenter();
    }
    m_targetPos = pos;
    result.linear = m_targetPos - m_thisPos;
    //velocity is along this direction, at full speed
    result.linear.Normalize();
    result.linear *= m_maxAcceleration;

    //face the direction
    //m_pObject->GetKinematic()->NewOrientation(result.linear);

    return result;
}


E2::Pursue::Pursue(float maxAcceleration, float maxPredictTime)
    : Seek(maxAcceleration)
    , m_maxPrediction{ maxPredictTime }
{
}

E2::SteeringOutput E2::Pursue::GetSteering(GameObject* pA, GameObject* pB)
{
    //Calculate the pursue target to delegate to seek
    //work out the distance to target
    m_thisPos = pA->GetTransform()->GetCenter();
    m_targetPos = pB->GetTransform()->GetCenter();

    auto direction = m_targetPos - m_thisPos;
    auto distance = direction.Magnitude();
    float prediction = 0;

    //current speed
    auto currentSpeed = pA->GetKinematic()->GetVelocity().Magnitude();

    //check if speed is reasonable
    if (currentSpeed <= distance / m_maxPrediction)
    {
        prediction = m_maxPrediction;
    }
    else
    {
        prediction = distance / currentSpeed;
    }

    //add together
    m_targetPos = m_targetPos + pB->GetKinematic()->GetVelocity() * prediction;

    //Delegate to Seek
    return Seek::GetSteering(pA, nullptr);
}
