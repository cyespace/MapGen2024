#include "Wiki.h"

#include <LuaEmbed.h>
#include <UIElement.h>
#include <UILabel.h>
#include <UIImage.h>
#include <GlobalFunctions.h>

constexpr const char* kCritterList = "Script/CritterList.lua";
constexpr const char* kWikiUIBackGround = "Script/UI/Wiki/Image_BackGround.lua";
constexpr const char* kWikiUICritterName = "Script/UI/Wiki/Label_CritterName.lua";
constexpr const char* kWikiUICritterId = "Script/UI/Wiki/Label_CritterId.lua";
constexpr const char* kWikiUICritterDestription = "Script/UI/Wiki/Label_CritterDescription.lua";
constexpr const char* kWikiUICritterImage = "Script/UI/Wiki/Image_Critter.lua";
constexpr const char* kWikiUIPrevButton = "Script/UI/Wiki/Button_Prev.lua";
constexpr const char* kWikiUINextButton = "Script/UI/Wiki/Button_Next.lua";

bool Wiki::Init()
{
    bool dataIsGood = LoadData(kCritterList);
    bool UIIsGood = InitUI();
    if (UIIsGood)
    {
        Close();
    }
    
    return dataIsGood && UIIsGood;
}

void Wiki::Open()
{
    m_pBackGround->SetVisable(true);
    m_pCritterImage->SetVisable(true);
    m_pCritterId->SetVisable(true);
    m_pCritterName->SetVisable(true);
    m_pCritterDescription->SetVisable(true);
    m_isOpen = true;

}

void Wiki::Close()
{
    m_pBackGround->SetVisable(false);
    m_pCritterImage->SetVisable(false);
    m_pCritterId->SetVisable(false);
    m_pCritterName->SetVisable(false);
    m_pCritterDescription->SetVisable(false);
    m_isOpen = false;
}

void Wiki::DiscoverCritter(int critterId)
{
    if (m_currentId != critterId)
    {
        m_wiki[critterId].m_isFound = true;
        m_currentId = critterId;

        dynamic_cast<E2::UILabel*>(m_pCritterId)->SetText(std::to_string(m_wiki[critterId].m_id));
        dynamic_cast<E2::UILabel*>(m_pCritterName)->SetText(m_wiki[critterId].m_name);
        dynamic_cast<E2::UILabel*>(m_pCritterDescription)->SetText(m_wiki[critterId].m_description);
        dynamic_cast<E2::UIImage*>(m_pCritterImage)->SetImage(m_wiki[critterId].m_texture);
    }
    Open();
}

bool Wiki::LoadData(const char* pWikiData)
{
    //lua index starts from 1, so add a dummy at index 0
    m_wiki.emplace_back();

    lua_State* pLuaState = luaL_newstate();
    luaL_openlibs(pLuaState);

    if (E2::CheckLua(pLuaState, (int)luaL_dofile(pLuaState, pWikiData)))
    {
        lua_getglobal(pLuaState, "CritterList");                    //(top)->[Critter List]
        size_t numberOfEntries = lua_rawlen(pLuaState, -1);

        //lua index starts from 1
        for (int i = 1; i <= numberOfEntries; ++i)
        {
            lua_pushnumber(pLuaState, i);                           //->[i],[Critter List],
            lua_gettable(pLuaState, -2);                            //->[critter info at i],[Critter List]

            CritterEntry critter;

            lua_pushstring(pLuaState, "id");                        //->[id],[critter info at i],[Critter List]
            lua_gettable(pLuaState, -2);                            //->[value of id],[critter info at i],[Critter List]
            critter.m_id = (int)lua_tointeger(pLuaState, -1);
            lua_pop(pLuaState, 1);                                   //->[critter info at i],[Critter List]

            lua_pushstring(pLuaState, "name");                      //->[name],[critter info at i],[Critter List]
            lua_gettable(pLuaState, -2);                            //->[value of name],[critter info at i],[Critter List]
            critter.m_name = lua_tostring(pLuaState, -1);
            lua_pop(pLuaState, 1);                                  //->[critter info at i],[Critter List]

            lua_pushstring(pLuaState, "description");               //->[description],[critter info at i],[Critter List]
            lua_gettable(pLuaState, -2);                            //->[value of description],[critter info at i],[Critter List]
            critter.m_description = lua_tostring(pLuaState, -1);
            lua_pop(pLuaState, 1);                                  //->[critter info at i],[Critter List]

            lua_pushstring(pLuaState, "texture");                   //->[texture],[critter info at i],[Critter List]
            lua_gettable(pLuaState, -2);                            //->[value of texture],[critter info at i],[Critter List]
            std::string texturePath = lua_tostring(pLuaState, -1);
            lua_pop(pLuaState, 2);                                  //->[Critter List]

            critter.m_texture = GetEngine().CreateTexture(texturePath.c_str());
            m_wiki.emplace_back(critter);
        }
        lua_close(pLuaState);
        m_wiki;
        return true;
    }
    lua_close(pLuaState);
    return false;
}

bool Wiki::InitUI()
{
    m_pBackGround = GetEngine().Lua_LoadUIElement(kWikiUIBackGround);
    m_pCritterImage = GetEngine().Lua_LoadUIElement(kWikiUICritterImage);
    m_pCritterId = GetEngine().Lua_LoadUIElement(kWikiUICritterId);
    m_pCritterName = GetEngine().Lua_LoadUIElement(kWikiUICritterName);
    m_pCritterDescription = GetEngine().Lua_LoadUIElement(kWikiUICritterDestription);

    dynamic_cast<E2::UILabel*>(m_pCritterId)->SetStyle(E2::UILabel::HorizontalAlign::Center,E2::UILabel::VerticalAlign::Middle,false);
    dynamic_cast<E2::UILabel*>(m_pCritterName)->SetStyle(E2::UILabel::HorizontalAlign::Center, E2::UILabel::VerticalAlign::Middle, false);
    dynamic_cast<E2::UILabel*>(m_pCritterDescription)->SetStyle(E2::UILabel::HorizontalAlign::Left, E2::UILabel::VerticalAlign::Top, true);

    return true;
}
