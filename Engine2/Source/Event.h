#pragma once
#include "EventType.h"
namespace E2
{
    struct KeyBoardEvent
    {
        char key = 0;
    };

    struct MouseEvent
    {
        int x = 0;
        int y = 0;
        int button = 0; //-1 = no click
    };

    struct Event
    {
        size_t m_eventType = 0;
        union
        {
            KeyBoardEvent m_keyBoardEvent;
            MouseEvent m_mouseEvent;
        };
    };

    
}