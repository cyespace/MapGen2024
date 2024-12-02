#include "pch.h"
#include "Engine.h"
#include <iostream>

E2::Device::Device()
    : m_pWindow{nullptr}
    , m_pRenderer{nullptr}
{
}

E2::Device::~Device()
{
    
}

bool E2::Device::Init(const GameSetting& info)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    m_pWindow = SDL_CreateWindow(info.m_gameName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        info.m_windowWidth, info.m_windowHeight, SDL_WINDOW_SHOWN);
    std::cout << "SDL Window ready.\n";

    m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED);
    std::cout << "SDL Renderer ready.\n";

    TTF_Init();
    std::cout << "SDL_ttf ready.\n";
    

    return true;
}

void E2::Device::ShutDown()
{
    TTF_Quit();
    std::cout << "SDL_ttf shuts down.\n";

    SDL_DestroyRenderer(m_pRenderer);
    std::cout << "SDL Renderer shuts down.\n";

    SDL_DestroyWindow(m_pWindow);
    std::cout << "SDL Window shuts down.\n";

    SDL_Quit();
}

