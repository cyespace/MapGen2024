#include "pch.h"
#include "Timer.h"
#include <chrono>

static std::chrono::time_point<std::chrono::high_resolution_clock> s_timePoint;

E2::Timer::Timer()
{
    s_timePoint = std::chrono::high_resolution_clock::now();
    m_duration = 0;
}

void E2::Timer::Tick()
{
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float, std::milli> duration = end - s_timePoint;
    s_timePoint = end;

    m_duration = duration.count() / 1000.f;
}
