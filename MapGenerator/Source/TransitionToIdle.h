#pragma once

#include <Transition.h>
namespace E2
{
    class GameObject;
}
class TransitionToIdle : public E2::Transition
{
public:
    TransitionToIdle(E2::GameObject* pObject)
        : Transition(pObject)
    {}
    virtual bool ToTransition() const override;
};