#pragma once
#include "GameObject.h"

namespace E2
{
    class Transition
    {
    protected:
        GameObject* m_pOwner = nullptr;
    public:
        Transition(GameObject* pObject)
            : m_pOwner{ pObject }
        {}
        virtual ~Transition() = default;

        virtual bool ToTransition() const = 0;
    };
}