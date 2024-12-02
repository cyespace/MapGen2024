#include "IdleState.h"
#include <Random.h>

//#include <iostream>

IdleState::IdleState(E2::GameObject* pGameObject, float minIdleTime, float maxIdleTime)
    : State(pGameObject)
    , m_minIdleTime{ minIdleTime }
    , m_maxIdleTime{ maxIdleTime }
{
}

void IdleState::Update(float deltaTime)
{
    m_timeLeft -= deltaTime;
    if (m_timeLeft < 0)
    {
        m_isFinished = true;
    }
}

void IdleState::OnEnter()
{
    m_isFinished = false;
    m_timeLeft =(float) E2::Rand::Random((int)m_minIdleTime, (int)m_maxIdleTime);
    //std::cout << "Obj " << m_pOwner->GetId() << " enter IdleState.\n";
}

void IdleState::OnExit()
{
    //std::cout << "Obj " << m_pOwner->GetId() << " exit IdleState.\n";
}
