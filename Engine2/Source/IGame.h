#pragma once
#include <string>
namespace E2
{
    struct GameSetting
    {
        std::string m_gameName{};
        int m_windowWidth = 0;
        int m_windowHeight = 0;
    };

    const GameSetting kDefaultSetting{ {"Default"},{1000},{1000}};
    
    class IGame
    {
    public:
        virtual ~IGame() = default;
        virtual bool Init() = 0;
        virtual void Update(float deltaTime) = 0;
        virtual void ShutDown() = 0;
        virtual const char* Config() = 0;
    };
}
