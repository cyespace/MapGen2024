#pragma once


namespace E2
{
    class Timer
    {
    private:
        float m_duration;
    public:
        Timer();
        ~Timer() = default;

        void Tick();
        float DeltaTime() const { return m_duration; }
    private:
    };
}
