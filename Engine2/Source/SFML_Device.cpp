#include "pch.h"
#include "Device.h"

#include <iostream>

bool E2::Device::Init()
{
    std::cout << "sfml init!\n";
    return true;
}

void E2::Device::DrawRect(int x, int y, int w, int h)
{
    std::cout << "sfml draws!\n";
}

E2::Texture E2::Device::GetTexture(const char* filePath)
{
    return Texture();
}

void E2::Device::PlaySound()
{
    std::cout << "sfml playing sound!\n";
}

void E2::Device::ShutDown()
{
    std::cout << "sfml quit!\n";
}
