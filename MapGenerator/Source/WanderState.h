#pragma once
#include <State.h>
#include <Vector2.h>

namespace E2
{
    class GameObject;
}

class WanderState : public E2::State
{
private:
    std::vector<E2::Vector2f> m_wayPoints;
    E2::Vector2f m_startPos;
    float m_wanderRadius = 0;
    float m_moveSpeed = 0;
    float m_currentLerp = 0;
    int m_currentWayPoint = 0;
public:
    WanderState(E2::GameObject* pGameObject, float radius, float speed);
    virtual void OnEnter()override;
    virtual void OnExit()override;
    virtual void Update(float deltaTime) override;

};