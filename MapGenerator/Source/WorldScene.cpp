#include "MapGenerator.h"
#include "WorldScene.h"
#include "WorldMapUIRect.h"
#include "WorldMap.h"
#include "TerraForm.h"
#include "Region.h"

#include <GlobalFunctions.h>
#include <EventType.h>
#include <CallBack.h>
#include <SimpleMath.h>
#include <LuaEmbed.h>
#include <UIImage.h>

#include <iostream>
#include <Assert.h>

WorldScene::WorldScene(WorldMap* pMap)
    :m_pWorldMap{pMap}
{
}

//TODO: don't destroy it twice
WorldScene::~WorldScene()
{
    if (!m_hasEnded)
    {
        End();
    }
}

bool WorldScene::Init()
{
    if (m_pWorldMap)
    {
        CreateWorldTexture();
        InitUI();

        std::cout << "World Scene Init.\n";
        std::cout << "World Scene: Map Size: " << m_worldMapTexture.dimension.x << ", " << m_worldMapTexture.dimension.y << '\n';
        std::cout << "World Scene: Map Seed: " << MapGenerator::Get().GetMapSeed() << "\n";
        return true;
    }
    else
    {
        std::cout << "Error: No world map!\n";
        return false;
    }
}

void WorldScene::Update(float deltaTime)
{
    InputCheck();
}

void WorldScene::End()
{
    m_pWorldMap = nullptr;
    GetEngine().RemoveListener(m_selectionRect, kMouseEvent);
    GetEngine().ClearUI();
    GetEngine().DestroyTexture(m_worldMapTexture);
    m_hasEnded = true;
    std::cout << "World Scene Ends.\n";
}

void WorldScene::InputCheck()
{
    if (GetEngine().IsKeyPressed(E2::Keyboard::Key::Escape))
    {
        MapGenerator::Get().Restart();
    }
}

void WorldScene::InitUI()
{
    auto windowW = E2::Engine::Get().GetWindowSize().x;
    auto windowH = E2::Engine::Get().GetWindowSize().y;

    auto* pMapImage = new E2::UIImage(m_worldMapTexture, E2::Rect{}, E2::Rect{0,0,windowW,windowH}
    , { E2::UICoordType::Percentage, 0.5f}, { E2::UICoordType::Percentage, 0.5f }
    , { E2::UICoordType::Percentage, 0.9f }, { E2::UICoordType::Percentage, 0.9f }
    , { E2::UICoordType::Percentage, 0.5f }, { E2::UICoordType::Percentage, 0.5f });
    GetEngine().AddUIElement(pMapImage);

    auto* pSelectionRect = new WorldMapUIRect(pMapImage->GetRealRegion(), E2::RedColor::kRed);
    pSelectionRect->SetDimension({ E2::UICoordType::Pixels, kViewRange}, { E2::UICoordType::Pixels, kViewRange});
    pSelectionRect->Reposition(0, 0, 0, 0);
    auto selectionRectCallBack = [](std::uintptr_t address)
    {
        E2::Rect rect{};
        WorldScene* pWorldScene = reinterpret_cast<WorldScene*>(address);
        pWorldScene->GotoLocalScene();
    };
    pSelectionRect->SetCallBack(new E2::CallBack(reinterpret_cast<std::uintptr_t>(this), selectionRectCallBack));
    m_selectionRect = pSelectionRect;

    GetEngine().AddUIElement(m_selectionRect);
    GetEngine().RegisterListener(m_selectionRect, kMouseEvent);
}

E2::Vector2 WorldScene::GetSelectionStart()
{
    return E2::Vector2(m_selectionRect->GetActualX(), (m_selectionRect->GetActualY()));
}

E2::Vector2 WorldScene::GetSelectionEnd()
{
    return E2::Vector2(m_selectionRect->GetActualX()+ m_selectionRect->GetActualW(), (m_selectionRect->GetActualY() + m_selectionRect->GetActualH()));
}

void WorldScene::CreateWorldTexture()
{
    GetEngine().CleanRenderer();

    auto drawHeightNoise = [](Tile* pTile)
    {
        uint8_t color = uint8_t(255.f * pTile->m_heightNoise);
        DrawPoint(pTile->m_x, pTile->m_y, { color,color,color,255 });
    };

    auto drawHeight = [](Tile* pTile)
    {
        uint8_t color = uint8_t(255.f * pTile->m_height);
        DrawPoint(pTile->m_x, pTile->m_y, { color,color,color,255 });
    };

    auto drawTemperatureNoise = [](Tile* pTile)
    {
        float hue = E2::Lerp(0, 230, 1.f - (pTile->m_temperatureNoise));
        E2::Color color = E2::Color_hsva(hue, 1, 1);
        DrawPoint(pTile->m_x, pTile->m_y, color);
    };

    auto drawBiome = [](Tile* pTile)
    {
        DrawPoint(pTile->m_x, pTile->m_y, pTile->m_biomeColor);
    };

    auto drawTreeNoise = [](Tile* pTile)
    {
        uint8_t color = uint8_t(255.f * pTile->m_treeNoise);
        DrawPoint(pTile->m_x, pTile->m_y, { color,color,color,255 });
    };

    auto drawWorld = [](Tile* pTile)
    {
        if (pTile->m_isWorldRoad)
        {
            DrawPoint(pTile->m_x, pTile->m_y, E2::RedColor::kPink);
        }
        else
        {
            DrawPoint(pTile->m_x, pTile->m_y, pTile->m_biomeColor);
        }
    };

    m_pWorldMap->ForEachTile(drawWorld);

    auto& townCenters = m_pWorldMap->GetTownCenters();
    int drawLength = 10;
    for (int index : townCenters)
    {
        auto* pTile = m_pWorldMap->GetTile(index);
        E2::Rect rect{ pTile->m_x - drawLength, pTile->m_y - drawLength, drawLength * 2, drawLength * 2 };
        DrawRect(rect, E2::RedColor::kRed);
    }


    m_worldMapTexture = GetEngine().ScreenCapture(0,0, m_pWorldMap->GetColumnCount(), m_pWorldMap->GetRowCount());
}

void WorldScene::TestDraw()
{
    //static auto collection = m_pWorldMap->GetTilesInRadius(8000, 20, true);
}

void WorldScene::DrawWorldMap()
{
    DrawTexture(m_worldMapTexture, nullptr, nullptr);
}

void WorldScene::GotoLocalScene()
{
    auto [xf, yf, wf, hf] = dynamic_cast<WorldMapUIRect*>(m_selectionRect)->GetSelectionRelativePosition();

    int x = (int)(xf * (float)kMapSizeX);
    int y = (int)(yf * (float)kMapSizeY);
    int w = (int)(wf * (float)kMapSizeX);
    int h = (int)(hf * (float)kMapSizeY);
    MapGenerator::Get().ViewLocal({x,y,w,h});
}
