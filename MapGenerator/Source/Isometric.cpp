#include "Isometric.h"

#include <FreeFunctions.h>
#include <Rect.h>

#include <iostream>

constexpr const char* kTileName = "Assets/Isometric/Basic2.png";
static E2::Texture s_texture;

IsometricTest& IsometricTest::Get()
{
    static IsometricTest instance;
    return instance;
}

IsometricTest::~IsometricTest()
{
}

bool IsometricTest::Init()
{
    GetEngine().LoadFile(kTileName);
    s_texture = GetEngine().CreateTexture(kTileName);

    GetEngine().RegisterListener(this, kMouseEvent);
    return true;
}

void IsometricTest::Update(float deltaTime)
{
    for (int y = 0; y < 20; ++y)
    {
        for (int x = 0; x < 20; ++x)
        {
            int yOffset = 0;
            if (x == m_selectX && y == m_selectY)
            {
                yOffset = 8;
            }

            float x1 = (float)x * 0.5f * 32.f + (float)y * (-0.5f) * 32.f;
            float y1 = (float)x * 0.25f * 32.f + (float)y * (0.25f) * 32.f;
            E2::Rect dest{x1 - 16 +360,y1 +180  - yOffset,32,32};
            DrawTexture(s_texture,nullptr, &dest);
        }
    }
}

const char* IsometricTest::Config()
{
    return nullptr;
}

void IsometricTest::OnNotify(E2::Event evt)
{
    if (evt.m_eventType = std::hash<std::string>{}(std::string(kMouseEvent)))
    {
        int x = evt.m_mouseEvent.x;
        int y = evt.m_mouseEvent.y;

        std::cout << "X= " << x << " ,Y= " << y << "\n";

        float x1 = (float)x / 32.f + (float)y / 16.f - 22.5f;
        float y1 = -(float)x / 32.f + (float)y / 16.f ;
        std::cout << "X1= " << x1 << " ,Y1= " << y1 << "\n";

        m_selectX = (int)x1;
        m_selectY = (int)y1;
    }
}
