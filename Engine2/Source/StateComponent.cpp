#include "pch.h"
#include "StateComponent.h"

E2::StateComponent::StateComponent(GameObject* pOwner)
    :Component(pOwner)
{
}

E2::StateComponent::~StateComponent()
{
    for (auto& [pState, transitionPairs] : m_transitionMap)
    {
        for (auto& transitionPair : transitionPairs)
        {
            delete transitionPair.first;
            transitionPair.first = nullptr;
        }
        delete pState; //hmm //BUG
        //*pState = nullptr;
    }
}

void E2::StateComponent::Update(float deltaTime)
{
    auto it = m_transitionMap.find(m_pCurrentState);
    if (it != m_transitionMap.end())
    {
        // loop through every transition for this state
        for (auto& transPair : it->second)
        {
            // check for transition
            if (transPair.first && transPair.first->ToTransition())
            {
                SetState(transPair.second);
                break;
            }
        }
    }

    if (m_pCurrentState)
    {
        m_pCurrentState->Update(deltaTime);
    }
}

void E2::StateComponent::SetState(State* pState)
{
    if (m_pCurrentState)
    {
        m_pCurrentState->OnExit();
    }
    m_pCurrentState = pState;
    m_pCurrentState->OnEnter();
}

void E2::StateComponent::AddState(State* pState, Transition* pTransition, State* pToState)
{
    if (pState)
    {
        m_transitionMap[pState].emplace_back(std::make_pair(pTransition, pToState));
    }
}
