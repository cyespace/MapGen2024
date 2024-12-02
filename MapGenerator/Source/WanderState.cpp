#include "WanderState.h"
#include "MapGenerator.h"
//#include "LocalScene.h"

#include <SimpleMath.h>

//#include <iostream>

WanderState::WanderState(E2::GameObject* pGameObject, float radius, float speed)
    :State(pGameObject)
    ,m_wanderRadius{radius}
    ,m_moveSpeed{ speed }
{
}

void WanderState::OnEnter()
{
    m_isFinished = false;
    m_wayPoints.clear();
    m_currentLerp = 0;
    m_currentWayPoint = 0;

    //get current position
    m_startPos = m_pOwner->GetPosition();
    //find a random near position
    //LocalScene* pScene = dynamic_cast<LocalScene*>(MapGenerator::Get().GetScene(Scene::SceneId::LocalScene));
    //m_wayPoints = pScene->BuildPath(m_startPos, pScene->GetRandomNearPositionInNav(m_startPos,m_wanderRadius));
    //std::cout << "Obj " << m_pOwner->GetId() << " enter WanderState.\n";
}

void WanderState::OnExit()
{
    //std::cout << "Obj " << m_pOwner->GetId() << " exit WanderState.\n";
}

void WanderState::Update(float deltaTime)
{
    if (m_currentWayPoint < m_wayPoints.size())
    {
        if (m_startPos.IsEqual(m_wayPoints[m_currentWayPoint],0.001f))
        {
            ++m_currentWayPoint;
            return;
        }
        float step = deltaTime * m_moveSpeed;
        m_currentLerp += step;
        m_pOwner->SetPosition(E2::Lerp(m_startPos, m_wayPoints[m_currentWayPoint], m_currentLerp));
        if (m_currentLerp > 1)
        {
            m_currentLerp = 0;
            //m_pOwner->SetPosition(m_wayPoints[m_currentWayPoint]);
            m_startPos = m_wayPoints[m_currentWayPoint];
            ++m_currentWayPoint;
        }
    }
    else
    {
        m_isFinished = true;
    }
}
