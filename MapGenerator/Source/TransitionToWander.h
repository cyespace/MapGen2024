#pragma once

#include <Transition.h>
namespace E2
{
    class GameObject;
}

class TransitionToWander : public E2::Transition
{
public:
    TransitionToWander(E2::GameObject* pObject)
        : Transition(pObject)
    {}
    virtual bool ToTransition() const override;
};