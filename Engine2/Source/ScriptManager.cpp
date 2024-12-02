#include "pch.h"
#include "Engine.h"

#include "UILabel.h"
#include "UIImage.h"
#include "UITextInput.h"
#include "UIButton.h"
#include "ScriptManager.h"

#include "EventType.h"

#include "LuaEmbed.h"

#include <iostream>

E2::ScriptManager::ScriptManager(E2::Engine* pEngine)
    :m_pEngine{ pEngine }
{
    m_pLua = luaL_newstate();
    luaL_openlibs(m_pLua);
}

E2::ScriptManager::~ScriptManager()
{
    lua_close(m_pLua);
}

bool E2::ScriptManager::LoadUIElements(const char* pScript)
{
    if (CheckLua(m_pLua, (int)luaL_dofile(m_pLua, pScript)))
    {
        lua_register(m_pLua,"CPP_LoadUIElement", Host_LoadUIElement);
        lua_getglobal(m_pLua, "LoadUIElements");
        if(lua_isfunction(m_pLua, -1))
        {
            if(CheckLua(m_pLua,lua_pcall(m_pLua, 0, 0, 0)))
            {
                return true;
            }
        }
    }

    return false;
}

E2::UIElement* E2::ScriptManager::LoadUIElement(const char* pScript)
{
    if (CheckLua(m_pLua,(int)luaL_dofile(m_pLua, pScript)))
    {
        std::string UIType;

        lua_getglobal(m_pLua, "UIElement");
        if (lua_istable(m_pLua, -1))
        {
            lua_pushstring(m_pLua, "Type");
            lua_gettable(m_pLua, -2);
            UIType = lua_tostring(m_pLua, -1);
            lua_pop(m_pLua, 1);

            if (UIType == "Label")
            {
                return LoadUILabel();
            }
            else if (UIType == "Image")
            {
                return LoadUIImage();
            }
            else if (UIType == "TextInput")
            {
                return LoadUITextInput();
            }
            else if (UIType == "Button")
            {
                return LoadUIButton();
            }
        }
    }
    return nullptr;
}

//static
int E2::ScriptManager::Host_LoadUIElement(lua_State* pState)
{
    std::string filePath = lua_tostring(pState,-1);
    E2::Engine::Get().Lua_LoadUIElement(filePath.c_str());
    
    return 0;
}

bool E2::ScriptManager::LoadFile(const char* pScript)
{
    if (CheckLua(m_pLua, (int)luaL_dofile(m_pLua, pScript)))
    {
        lua_getglobal(m_pLua, "Files");
        if (lua_istable(m_pLua, -1))
        {
            lua_pushnil(m_pLua);
            
            while (lua_next(m_pLua, -2))
            {
                lua_pushvalue(m_pLua, -2);
                
                const char* value = lua_tostring(m_pLua, -2);
                [[maybe_unused]]bool success = m_pEngine->LoadFile(value);
                assert(success && "LoadFile " && value);
                
                lua_pop(m_pLua, 2);
            }
        }
    }
    return false;
}

bool E2::ScriptManager::LoadGameSettings(const char* pScript, GameSetting& gameSetting)
{
    if (!pScript)
    {   
        std::cout << "Game Config File Is Null.\n";
        return false;
    }

    if (CheckLua(m_pLua, (int)luaL_dofile(m_pLua, pScript)))
    {
        lua_getglobal(m_pLua, "GameSetting");
        if (lua_istable(m_pLua, -1))
        {
            lua_pushstring(m_pLua, "Title");
            lua_gettable(m_pLua, -2);
            gameSetting.m_gameName = lua_tostring(m_pLua, -1);
            lua_pop(m_pLua, 1);

            lua_pushstring(m_pLua, "WindowWidth");
            lua_gettable(m_pLua, -2);
            gameSetting.m_windowWidth = (int)lua_tonumber(m_pLua, -1);
            lua_pop(m_pLua, 1);

            lua_pushstring(m_pLua, "WindowHeight");
            lua_gettable(m_pLua, -2);
            gameSetting.m_windowHeight = (int)lua_tonumber(m_pLua, -1);
            lua_pop(m_pLua, 1);

            return true;
        }
    }
    return false;
}

E2::UIElement* E2::ScriptManager::LoadUILabel()
{
    UILabelData data;
    lua_pushstring(m_pLua, "Content");
    lua_gettable(m_pLua, -2);
    data.content = lua_tostring(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "IsChild");
    lua_gettable(m_pLua, -2);
    bool isChild = lua_toboolean(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "Font");
    lua_gettable(m_pLua, -2);
    data.font = lua_tostring(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "FontSize");
    lua_gettable(m_pLua, -2);
    data.fontSize = (int)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "Color");
    lua_gettable(m_pLua, -2);

    lua_pushnumber(m_pLua, 1);
    lua_gettable(m_pLua, -2);
    data.r = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushnumber(m_pLua, 2);
    lua_gettable(m_pLua, -2);
    data.g = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushnumber(m_pLua, 3);
    lua_gettable(m_pLua, -2);
    data.b = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pop(m_pLua, 1);

    LoadPositionInfo(data);

    auto pElement = new E2::UILabel(m_pEngine->CreateFont(data.font.c_str(), data.fontSize), data.content, E2::Color{ data.r,data.g,data.b });
    pElement->SetPositionInfo(data);
    if (!isChild)
    {
        pElement->Reposition(0, 0, m_pEngine->GetWindowSize().x, m_pEngine->GetWindowSize().y);
        m_pEngine->AddUIElement(pElement);
    }
    return pElement;
}

E2::UIElement* E2::ScriptManager::LoadUIImage()
{
    UIImageData data;
    lua_pushstring(m_pLua, "Texture");
    lua_gettable(m_pLua, -2);
    data.texture = lua_tostring(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "SourceX");
    lua_gettable(m_pLua, -2);
    data.srcRect.x = (int)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "SourceY");
    lua_gettable(m_pLua, -2);
    data.srcRect.y = (int)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "SourceW");
    lua_gettable(m_pLua, -2);
    data.srcRect.w = (int)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "SourceH");
    lua_gettable(m_pLua, -2);
    data.srcRect.h = (int)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "DestX");
    lua_gettable(m_pLua, -2);
    data.destRect.x = (int)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "DestY");
    lua_gettable(m_pLua, -2);
    data.destRect.y = (int)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "DestW");
    lua_gettable(m_pLua, -2);
    data.destRect.w = (int)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "DestH");
    lua_gettable(m_pLua, -2);
    data.destRect.h = (int)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "Slice");
    lua_gettable(m_pLua, -2);
    data.isSliced = (bool)lua_toboolean(m_pLua, -1);
    lua_pop(m_pLua, 1);

    if (data.isSliced)
    {
        lua_pushstring(m_pLua, "SliceLeft");
        lua_gettable(m_pLua, -2);
        data.leftSlice = (float)lua_tonumber(m_pLua, -1);
        lua_pop(m_pLua, 1);

        lua_pushstring(m_pLua, "SliceRight");
        lua_gettable(m_pLua, -2);
        data.rightSlice = (float)lua_tonumber(m_pLua, -1);
        lua_pop(m_pLua, 1);

        lua_pushstring(m_pLua, "SliceTop");
        lua_gettable(m_pLua, -2);
        data.topSlice = (float)lua_tonumber(m_pLua, -1);
        lua_pop(m_pLua, 1);

        lua_pushstring(m_pLua, "SliceBottom");
        lua_gettable(m_pLua, -2);
        data.bottomSlice = (float)lua_tonumber(m_pLua, -1);
        lua_pop(m_pLua, 1);
    }

    LoadPositionInfo(data);

    auto pImage = new E2::UIImage(m_pEngine->CreateTexture(data.texture.c_str())
        , { data.srcRect }
        , { data.destRect }
        , { E2::UICoordType::Percentage,data.posX }
        , { E2::UICoordType::Percentage,data.posY }
        , { E2::UICoordType::Percentage,data.dimensionX }
        , { E2::UICoordType::Percentage,data.dimensionY }
        , { E2::UICoordType::Percentage,data.originX }
        , { E2::UICoordType::Percentage,data.originY });

    if (data.isSliced)
    {
        pImage->SetSlice({ E2::UICoordType::Percentage,data.leftSlice }
                        , { E2::UICoordType::Percentage,data.rightSlice }
                        , { E2::UICoordType::Percentage,data.topSlice }
                        , { E2::UICoordType::Percentage,data.bottomSlice });
    }
    m_pEngine->AddUIElement(pImage);

    return pImage;
}

E2::UIElement* E2::ScriptManager::LoadUITextInput()
{
    UITextInputData data;
    lua_pushstring(m_pLua, "Font");
    lua_gettable(m_pLua, -2);
    data.font = lua_tostring(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "FontSize");
    lua_gettable(m_pLua, -2);
    data.fontSize = (int)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "TextColor");
    lua_gettable(m_pLua, -2);

    lua_pushnumber(m_pLua, 1);
    lua_gettable(m_pLua, -2);
    data.textR = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushnumber(m_pLua, 2);
    lua_gettable(m_pLua, -2);
    data.textG = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushnumber(m_pLua, 3);
    lua_gettable(m_pLua, -2);
    data.textB = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "BackGroundColor");
    lua_gettable(m_pLua, -2);

    lua_pushnumber(m_pLua, 1);
    lua_gettable(m_pLua, -2);
    data.backGroundR = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushnumber(m_pLua, 2);
    lua_gettable(m_pLua, -2);
    data.backGroundG = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushnumber(m_pLua, 3);
    lua_gettable(m_pLua, -2);
    data.backGroundB = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pop(m_pLua, 1);

    LoadPositionInfo(data);

    auto* pElement = new E2::UITextInput(m_pEngine->CreateFont(data.font.c_str(), data.fontSize)
        , E2::Color{ data.textR,data.textG,data.textB }, E2::Color{data.backGroundR,data.backGroundG,data.backGroundB });
    pElement->SetPositionInfo(data);
    pElement->Reposition(0, 0, m_pEngine->GetWindowSize().x, m_pEngine->GetWindowSize().y);
    m_pEngine->AddUIElement(pElement);
    m_pEngine->RegisterListener(pElement,kKeyBoardEvent);
    return pElement;
}

E2::UIElement* E2::ScriptManager::LoadUIButton()
{
    E2::UIButtonData buttonData;

    lua_pushstring(m_pLua, "UpColor");
    lua_gettable(m_pLua, -2);

    lua_pushnumber(m_pLua, 1);
    lua_gettable(m_pLua, -2);
    buttonData.upColor.r =(uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushnumber(m_pLua, 2);
    lua_gettable(m_pLua, -2);
    buttonData.upColor.g = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushnumber(m_pLua, 3);
    lua_gettable(m_pLua, -2);
    buttonData.upColor.b = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "DownColor");
    lua_gettable(m_pLua, -2);

    lua_pushnumber(m_pLua, 1);
    lua_gettable(m_pLua, -2);
    buttonData.downColor.r = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushnumber(m_pLua, 2);
    lua_gettable(m_pLua, -2);
    buttonData.downColor.g = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushnumber(m_pLua, 3);
    lua_gettable(m_pLua, -2);
    buttonData.downColor.b = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "HoverColor");
    lua_gettable(m_pLua, -2);

    lua_pushnumber(m_pLua, 1);
    lua_gettable(m_pLua, -2);
    buttonData.hoverColor.r = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushnumber(m_pLua, 2);
    lua_gettable(m_pLua, -2);
    buttonData.hoverColor.g = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushnumber(m_pLua, 3);
    lua_gettable(m_pLua, -2);
    buttonData.hoverColor.b = (uint8_t)lua_tointeger(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pop(m_pLua, 1);

    LoadPositionInfo(buttonData);

    auto* pButton = new E2::UIButton(buttonData.upColor, buttonData.downColor, buttonData.hoverColor);
    pButton->SetPositionInfo(buttonData);

    lua_pushstring(m_pLua, "Child");
    lua_gettable(m_pLua, -2);
    std::string child = lua_tostring(m_pLua, -1);
    lua_pop(m_pLua, 1);

    if (!child.empty())
    {
        auto* pLabel = LoadUIElement(child.c_str());
        pButton->AddChild(pLabel);
    }
    pButton->Reposition(0, 0, m_pEngine->GetWindowSize().x, m_pEngine->GetWindowSize().y);

    m_pEngine->RegisterListener(pButton, kMouseEvent);
    m_pEngine->AddUIElement(pButton);
    return pButton;
}

void E2::ScriptManager::LoadPositionInfo(UIElementData& data)
{
    lua_pushstring(m_pLua, "PositionX");
    lua_gettable(m_pLua, -2);
    data.posX = (float)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "PositionY");
    lua_gettable(m_pLua, -2);
    data.posY = (float)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "OriginX");
    lua_gettable(m_pLua, -2);
    data.originX = (float)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "OriginY");
    lua_gettable(m_pLua, -2);
    data.originY = (float)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "DimensionX");
    lua_gettable(m_pLua, -2);
    data.dimensionX = (float)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);

    lua_pushstring(m_pLua, "DimensionY");
    lua_gettable(m_pLua, -2);
    data.dimensionY = (float)lua_tonumber(m_pLua, -1);
    lua_pop(m_pLua, 1);
}
