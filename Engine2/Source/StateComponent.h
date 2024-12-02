#pragma once
#include "Component.h"
#include "State.h"
#include "Transition.h"
#include <unordered_map>
namespace E2
{
    class StateComponent : public Component
    {
        using TransitionPair = std::pair<Transition*, State*>;
    private:
        std::unordered_map<State*, std::vector<TransitionPair>> m_transitionMap;
        State* m_pCurrentState = nullptr;
    public:
        StateComponent(GameObject* pOwner);
        virtual ~StateComponent();
        virtual void Update(float deltaTime) override;
        void SetState(State* pState);
        void AddState(State* pState, Transition* pTransition, State* pToState);
        bool IsCurrentFinished()const { return m_pCurrentState->IsFinished(); }
    };
}
