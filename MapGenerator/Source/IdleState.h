#pragma once

#include <State.h>

namespace E2
{
    class GameObject;
}

class IdleState : public E2::State
{
    float m_minIdleTime = 0;
    float m_maxIdleTime = 0;
    float m_timeLeft = 0;
public:
    IdleState(E2::GameObject* pGameObject, float minIdleTime, float maxIdleTime);
    virtual void Update(float deltaTime) override;
    virtual void OnEnter()override;
    virtual void OnExit()override;

};
