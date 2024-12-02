#pragma once
#include "Event.h"

namespace E2
{
    class EventListener
    {
    public:
        virtual void OnNotify(Event evt) {}
    };
}
