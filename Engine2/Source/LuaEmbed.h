#pragma once
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}


namespace E2
{
    bool CheckLua(lua_State* pState, int errorCode);
    void PrintElementTypeOnStack(lua_State* pState, int showHowManyElement);
}
