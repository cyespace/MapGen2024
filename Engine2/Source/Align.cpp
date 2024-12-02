#include "pch.h"
#include "Align.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "KinematicComponent.h"
#include "SimpleMath.h"

E2::Align::Align(float maxRotation, float maxAngularAcceleration, float slowRadius, float arriveRadius, float timeToTarget)
    : m_maxRotation{ maxRotation }
    , m_maxAngularAcceleration{ maxAngularAcceleration }
    , m_slowRadius{ slowRadius }
    , m_arriveRadius{ arriveRadius }
    , m_timeToArrive{ timeToTarget }
{
}

E2::SteeringOutput E2::Align::GetSteering(GameObject* pA, GameObject* pB)
{
    SteeringOutput result;
    if (pA)
    {
        m_thisOrientation = pA->GetTransform()->GetRotation();
    }
    if (pB)
    {
        m_targetOrientation = pB->GetTransform()->GetRotation();
    }
    auto rotation = m_targetOrientation - m_thisOrientation;
    float targetRotation = 0;
    //map the rotation to (-pi,pi)
    rotation = fmod(rotation, kPi);
    assert(rotation <= kPi);
    assert(rotation >= -kPi);
    auto rotationSize = abs(rotation);

    //check
    if (rotationSize < m_arriveRadius)
    {
        return result;
    }

    //outside the slowRadius
    if (rotationSize > m_slowRadius)
    {
        targetRotation = m_maxRotation;
    }
    else
    {
        targetRotation = m_maxRotation * rotationSize / m_slowRadius;
    }
    //final target rotation combines speed and direction
    targetRotation *= (rotation / rotationSize);

    //acceleration tries to get to the target rotation
    result.angular = targetRotation - m_thisOrientation;
    result.angular /= m_timeToArrive;

    //Check if acceleration is too great.
    auto angularAccleration = abs(result.angular);
    if (angularAccleration > m_maxAngularAcceleration)
    {
        result.angular /= angularAccleration;
        result.angular *= m_maxAngularAcceleration;
    }
    //result.angular = 0.01f;
    return result;
}

E2::Face::Face(float maxRotation, float maxAngularAcceleration, float slowRadius, float arriveRadius, float timeToTarget)
    : Align(maxRotation, maxAngularAcceleration, slowRadius, arriveRadius, timeToTarget)
{
}

E2::SteeringOutput E2::Face::GetSteering(GameObject* pA, GameObject* pB)
{
    auto targetPos = pB->GetTransform()->GetPosition();
    auto characterPos = pA->GetTransform()->GetPosition();
    auto direction = targetPos - characterPos;

    // Check for zero direction
    if (direction.Magnitude2() == 0)
    {
        return SteeringOutput();
    }

    //Delegate to align
    m_targetOrientation = atan2(direction.x, direction.y);
    pA->SetRotation(m_targetOrientation);
    return SteeringOutput();
    //TODO: Fix this align
    //return Align::GetSteering(pA,nullptr);
}

E2::LookWhereYoureGoing::LookWhereYoureGoing(float maxRotation, float maxAngularAcceleration, float slowRadius, float arriveRadius, float timeToTarget)
    : Align(maxRotation, maxAngularAcceleration, slowRadius, arriveRadius, timeToTarget)
{
}

E2::SteeringOutput E2::LookWhereYoureGoing::GetSteering(GameObject* pA, GameObject* pB)
{
    //calculate target
    auto velocity = pA->GetKinematic()->GetVelocity();
    if (velocity.Magnitude2() == 0)
    {
        return SteeringOutput();
    }

    //set the target based on velocity
    m_targetOrientation = atan2(velocity.x, velocity.y);
    pA->SetRotation(m_targetOrientation);
    return SteeringOutput();
    //TODO: Fix this align
    //return Align::GetSteering(pA,nullptr);
}
