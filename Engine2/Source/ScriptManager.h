#pragma once
struct lua_State;

namespace E2
{
    class Engine;
    class UIElement;
    struct UIElementData;
    struct GameSetting;
    class ScriptManager
    {
    private:
        lua_State* m_pLua = nullptr;
        Engine* m_pEngine = nullptr;
    public:
        ScriptManager(E2::Engine* pEngine);
        ~ScriptManager();

        bool LoadUIElements(const char* pScript);
        UIElement* LoadUIElement(const char* pScript);
        static int Host_LoadUIElement(lua_State* pState);
        bool LoadFile(const char* pScript);
        bool LoadGameSettings(const char* pScript, GameSetting& gameSetting);
    private:
        UIElement* LoadUILabel();
        UIElement* LoadUIImage();
        UIElement* LoadUITextInput();
        UIElement* LoadUIButton();  //TODO:lua user data for callbacks
        void LoadPositionInfo(UIElementData& data);
    };
}