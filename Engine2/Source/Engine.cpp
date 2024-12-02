#include "pch.h"
#include "Engine.h"
#include "HashString.h"
#include "Random.h"
#include "LuaEmbed.h"
#include "UILabel.h"
#include <iostream>
#include <string>
#include <ctime>




E2::Engine::~Engine()
{
    m_device.ShutDown();
}

bool E2::Engine::Init(const char* pGameConfig)
{
    std::cout << "Engine Init\n";
    bool ready = false;

    if(!LoadGameSettings(pGameConfig))
    {
        std::cout << "Failed to load game config. Using default values to init engine\n";
        ready = m_device.Init(kDefaultSetting);
    }
    else if (m_gameSetting.m_windowWidth == 0 || m_gameSetting.m_windowHeight == 0)
    {
        std::cout << "Impossible game configuration. Using default values to init engine\n";
        ready = m_device.Init(kDefaultSetting);
    }
    else
    {
        ready = m_device.Init(m_gameSetting);
    }
    if (Rand::GetSeed() == 0)
    {
        Rand::SetSeed(std::time(nullptr));
    }
    return ready;
}

void E2::Engine::Update(IGame& game,void(IGame::* pGameUpdate)(float),float deltaTime)
{
    //m_eventManger.Update();
    (game.*pGameUpdate)(deltaTime);
    m_gameObjectManager.Update(deltaTime);
    m_UIManager.Update();

    if (m_autoDrawGameObjects)
    {
        m_gameObjectManager.RenderGameObjects();
    }
    
    if (m_showUI)
    {
        m_UIManager.Draw();
    }
}

bool E2::Engine::Run(IGame& game)
{
    if (!(Init(game.Config())&& game.Init()))
    {
        return false;
    }

    std::cout << "Engine Runs Game\n";

    m_timer.Tick();
    while (!m_quit)
    {
        m_device.ProcessInput();
        ShowEngineStatus(m_timer.DeltaTime());

        if (m_canUpdate)
        {
            Update(game, &IGame::Update, m_timer.DeltaTime());
            m_device.Render();
        }
        if (m_manuallyUpdate && m_canUpdate)
        {
            m_canUpdate = false;
        }
        m_keyboard.NextFrame();
        m_mouse.NextFrame();
        m_timer.Tick();
    }

    game.ShutDown();
    ShutDown();
    return true;
}

bool E2::Engine::PointInRect(int x, int y, Rect& rect)
{
    return m_device.PointInRect(x,y,rect.x,rect.y,rect.w,rect.h);
}

E2::Vector2 E2::Engine::GetWindowSize()
{
    E2::Vector2 vec2;
    m_device.GetWindowSize(vec2.x, vec2.y);
    return vec2;
}

void E2::Engine::ToggleManuallyUpdate()
{
    m_manuallyUpdate = !m_manuallyUpdate;
}

void E2::Engine::PrintFPS(float deltaTime)
{
    static int frames = 0;
    static float time = 0;
    ++frames;
    time += deltaTime;
    if (time >= 1.f)
    {
        std::cout << "FPS = " << frames << '\n';
        time = 0;
        frames = 0;
    }
}

void E2::Engine::DestroyTexture(E2::Texture& texture)
{
    m_device.DestroyTexture(texture);
}

void E2::Engine::DrawRect(const Rect& rect, const Color& color)
{
    m_device.DrawRect(rect.x,rect.y,rect.w,rect.h,color.r,color.g,color.b,color.a);
}

void E2::Engine::DrawRectOutline(const Rect& rect, const Color& color)
{
    m_device.DrawRectOutline(rect.x, rect.y, rect.w, rect.h, color.r, color.g, color.b, color.a);
}

void E2::Engine::DrawLine(const Vector2& vec1, const Vector2& vec2, const Color& color)
{
    m_device.DrawLine(vec1.x,vec1.y,vec2.x,vec2.y, color.r, color.g, color.b, color.a);
}

void E2::Engine::DrawImage(const char* path)
{
    auto hash = HashString(path);
    auto p = m_resourceManager.GetResource(hash);
    if (p)
    {
        auto size = m_resourceManager.GetResourceSize(hash);
        DrawImageFromMem(p, size, nullptr, nullptr);
    }
}

void E2::Engine::DrawImageFromMem(std::byte* pSource, size_t size, Rect* pSrc, Rect* pDest)
{
    m_device.DrawImageFromMem(pSource,size,pSrc,pDest);
}

void E2::Engine::DrawTexture(Texture& texture, Rect* pSrc, Rect* pDest)
{
    m_device.DrawTexture(texture, pSrc, pDest);
}

void E2::Engine::DrawPoint(int x, int y, const Color& color)
{
    m_device.DrawPoint(x ,y , color.r, color.g, color.b, color.a);
}

bool E2::Engine::LoadFile(const char* path)
{
    return m_resourceManager.Load(path);
}

std::byte* E2::Engine::GetResource(const char* path)
{
    return m_resourceManager.GetResource(HashString(path));
}

size_t E2::Engine::GetResourceSize(const char* path)
{
    return m_resourceManager.GetResourceSize(HashString(path));
}

E2::Texture E2::Engine::CreateTexture(const char* path)
{
    auto id = HashString(path);
    auto texture = m_resourceManager.GetTexture(id);

    if (texture.pTexture)
    {
        return texture;
    }
    else
    {
        auto* pResource = GetResource(path);
        if (pResource)
        {
            auto size = GetResourceSize(path);
            TextureType type = m_resourceManager.GetTextureType(id);
            auto newtexture = m_device.CreateTexture(pResource, size, type);
            m_resourceManager.SaveTexture(id, newtexture);
            return newtexture;
        }
        else
        {
            assert(false && "E2::Engine::CreateTexture: cannot find resource to create texture\n");
        }
        return {};
    }
}

void E2::Engine::RenderNow()
{
    m_device.ForceRender();
}

void E2::Engine::CleanRenderer()
{
    m_device.CleanRenderer();
}

E2::Texture E2::Engine::ScreenCapture(int x, int y, int w, int h)
{
    return m_device.ScreenCapture(x,y,w,h);
}

E2::Font E2::Engine::CreateFont(const char* path, int height)
{
    auto* pResource = GetResource(path);
    if (pResource)
    {
        auto size = GetResourceSize(path);
        auto pNewFont = m_device.CreateFont(pResource, size, height);
        return pNewFont;
    }
    return {};
}

E2::Texture E2::Engine::CreateTextTexture(Font& font, std::string& text, Color color)
{
    auto newtexture = m_device.CreateTextTexture(font, text, color.r, color.g, color.b, color.a);
    return newtexture;
}

void E2::Engine::Notify(Event evt)
{
    m_eventManger.Notify(evt);
}

void E2::Engine::AddUIElement(UIElement* pElement)
{
    m_UIManager.AddElement(pElement);
}

E2::UIElement* E2::Engine::GetUIElement(const char* pName)
{
    return m_UIManager.GetElement(pName);
}

E2::UIElement* E2::Engine::Lua_LoadUIElement(const char* pPath)
{
    return m_scriptManager.LoadUIElement(pPath);
}

bool E2::Engine::LoadUIElements(const char* pPath)
{
    bool isGood = m_scriptManager.LoadUIElements(pPath);
    return isGood;
}

bool E2::Engine::LoadFilesFromScript(const char* pPath)
{
    m_scriptManager.LoadFile(pPath);
    return false;
}

bool E2::Engine::LoadGameSettings(const char* pPath)
{
    bool isGood = m_scriptManager.LoadGameSettings(pPath, m_gameSetting);
    return isGood;
}

void E2::Engine::ShutDown()
{
    m_resourceManager.DestroyAll();
    std::cout << "Engine Shuts Down\n";
}

void E2::Engine::ShowEngineStatus(float deltaTime)
{
    bool test = m_keyboard.IsKeyPressed(Keyboard::Key::F1);
    if (test)
    {
        m_showStatus = !m_showStatus;
    }

    if (m_showStatus)
    {
        PrintFPS(deltaTime);
    }
}

E2::Engine& E2::Engine::Get()
{
    static Engine instance;
    return instance;
}
