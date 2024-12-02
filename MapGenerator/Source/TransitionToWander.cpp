#include "TransitionToWander.h"
#include <StateComponent.h>

bool TransitionToWander::ToTransition() const
{
    auto* pComp = m_pOwner->GetComponent(E2::ComponentType::StateMachine);
    if (pComp)
    {
        auto* pStateMachine = dynamic_cast<E2::StateComponent*>(pComp);
        if (pStateMachine)
        {
            if (pStateMachine->IsCurrentFinished())
            {
                return true;
            }
        }
    }
    return false;
}
