#pragma once
#include <cstdint>

namespace E2
{
    struct CallBack
    {
        std::uintptr_t m_classAddress = 0;
        void (*m_pFunc)(std::uintptr_t) = nullptr;

        CallBack() = default;
        CallBack(std::uintptr_t address, void (*pFunc)(std::uintptr_t))
            : m_classAddress{ address }
            , m_pFunc{ pFunc }
        {}
        void Call()
        {
            m_pFunc(m_classAddress);
        }
    };
}
