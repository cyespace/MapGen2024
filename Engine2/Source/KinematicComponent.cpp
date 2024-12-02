#include "pch.h"
#include "KinematicComponent.h"
#include "TransformComponent.h"
#include "GameObject.h"
#include "Steering.h"
#include <cmath>



E2::KinematicComponent::KinematicComponent(GameObject* pOwner)
    : Component{ pOwner }
    , m_velocity{0,0}
{
}

void E2::KinematicComponent::Update(float deltaTime)
{
    //update position and orientation
    m_position += m_velocity * deltaTime;
    m_orientation += m_rotation * deltaTime;
    //update the velocity and rotation
    m_velocity += m_linearAcceleration * deltaTime;
    m_rotation += m_angularAcceleration * deltaTime;

    //check speed
    if (m_velocity.Magnitude2() > m_speedLimit * m_speedLimit)
    {
        m_velocity.Normalize();
        m_velocity *= m_speedLimit;
    }

    //sync transform

}

float E2::KinematicComponent::NewOrientation(Vector2f velocity)
{
    if (velocity.Magnitude2() > 0)
    {
        //SDL Y goes downwards
        m_orientation = (float)atan2( velocity.x, velocity.y);
        return m_orientation;
    }
    return m_orientation;
}

void E2::KinematicComponent::ApplyAcceleration(SteeringOutput steering)
{
    m_linearAcceleration = steering.linear;
    m_angularAcceleration = steering.angular;
}

void E2::KinematicComponent::ApplyKinematic(SteeringOutput steering)
{
    m_velocity = steering.linear;
    m_rotation = steering.angular;
}


void E2::KinematicComponent::Stop()
{
    m_velocity.Clear();
    m_rotation = 0;
    m_linearAcceleration.Clear();
    m_angularAcceleration = 0;
}
