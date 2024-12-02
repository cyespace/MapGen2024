#include "pch.h"
#include "VelocityMatch.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "KinematicComponent.h"

E2::VelocityMatch::VelocityMatch(float maxAcceleration, float timeToTarget)
    : m_maxAcceleration{ maxAcceleration }
    , m_timeToTarget{ timeToTarget }
{

}

E2::SteeringOutput E2::VelocityMatch::GetSteering(GameObject* pA, GameObject* pB)
{
    SteeringOutput result;

    //Acceleration tries to get to the target velocity
    auto startVelocity = pA->GetKinematic()->GetVelocity();
    auto endVelocity = pB->GetKinematic()->GetVelocity();
    result.linear = endVelocity - startVelocity;
    result.linear /= m_timeToTarget;

    //check if accelerating too fast
    if (result.linear.Magnitude() > m_maxAcceleration)
    {
        result.linear.Normalize();
        result.linear *= m_maxAcceleration;
    }

    return result;
}

E2::Arrive::Arrive(float maxSpeed
    , float maxAcceleration
    , float slowRadius
    , float arriveRadius
    , float timeToTarget)
{
    m_maxSpeed = maxSpeed;
    m_maxAcceleration = maxAcceleration;

    m_slowRadius = slowRadius;
    m_arriveRadius = arriveRadius;

    m_timeToTarget = timeToTarget;
}

E2::SteeringOutput E2::Arrive::GetSteering(GameObject* pA, GameObject* pB)
{
    return GetSteering(pA, pB->GetTransform()->GetCenter());
}

E2::SteeringOutput E2::Arrive::GetSteering(GameObject* pA, const E2::Vector2f pos)
{
    SteeringOutput result;

    auto startPos = pA->GetTransform()->GetCenter();
    auto endPos = pos;
    auto direction = endPos - startPos;
    auto distance = direction.Magnitude();
    float targetSpeed = 0;
    //check if we reach the target 
    if (distance < (m_arriveRadius))
    {
        //m_pObject->GetKinematic()->Stop();
        return result;
    }

    //if outside the slowRadius, move at max speed
    if (distance > m_slowRadius)
    {
        targetSpeed = m_maxSpeed;
    }
    else
    {
        targetSpeed = m_maxSpeed * distance / m_slowRadius;
    }
    //target velocity combines speed and direction
    auto targetVelocity = direction;
    targetVelocity.Normalize();
    targetVelocity *= targetSpeed;

    //Acceleration 
    result.linear = targetVelocity - pA->GetKinematic()->GetVelocity();
    result.linear /= m_timeToTarget;

    //check if Acceleration is too fast
    if (result.linear.Magnitude() > (m_maxAcceleration))
    {
        result.linear.Normalize();
        result.linear *= m_maxAcceleration;
    }

    //face to the direction we want to move
    //m_pObject->GetKinematic()->NewOrientation(result.linear);

    return result;
}
