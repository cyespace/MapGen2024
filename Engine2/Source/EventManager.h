#pragma once

#include "EventListener.h"
#include <vector>
#include <unordered_map>
namespace E2
{
    class EventManager
    {
    private:
        // TODO:DOESN'T WORK 
        //std::vector<std::pair<UIElement*, void(UIElement::*)(Event*)>> m_UIEventListenerTest;
        // other type of listeners..
        bool m_reset = false;    //hack

        std::unordered_map<size_t, std::vector<EventListener*>> m_eventListeners;



    public:
        EventManager();
        ~EventManager() = default;
        void AddListener(EventListener* pListener, const char* pEventType);
        void RemoveListener(EventListener* pListener, const char* pEventType);
        void ClearListener();
        void Notify(Event evt);
        void Update();

    };
}