#pragma once
#include "GameObject.h"
namespace E2
{
    class State
    {
    protected:
        GameObject* m_pOwner = nullptr;
        bool m_isFinished = false;
    public:
        State(GameObject* pOwner)
            : m_pOwner(pOwner)
        { }
        virtual ~State() { }
        virtual void OnEnter() { }
        virtual void OnExit() { }
        virtual void Update(float deltaTime) { }
        bool IsFinished()const { return m_isFinished; }
    protected:
        GameObject* GetOwner() const { return m_pOwner; }
    };
}