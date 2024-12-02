#include "pch.h"
#include "LuaEmbed.h"

#include <iostream>
bool E2::CheckLua(lua_State* pState, int errorCode)
{
    if (errorCode != LUA_OK)
    {
        const char* errormsg = lua_tostring(pState, -1);
        std::cout << errormsg << '\n';
        return false;
    }
    return true;
}

void E2::PrintElementTypeOnStack(lua_State* pState, int showHowManyElement)
{
    auto typeNumberToString = [](int num)-> std::string
    {
        switch (num)
        {
        case LUA_TNONE: return "NONE";
        case LUA_TNIL: return "NIL";
        case LUA_TBOOLEAN: return "bool";
        case LUA_TLIGHTUSERDATA: return "light user data";
        case LUA_TNUMBER: return "number";
        case LUA_TSTRING: return "string";
        case LUA_TTABLE: return "table";
        case LUA_TFUNCTION: return "function";
        case LUA_TUSERDATA: return "user data";
        case LUA_TTHREAD: return "thread";
        default: return "error type";
        }
    };


    std::cout << "-------Lua Stack------\n";
    for (int i = 1; i <= showHowManyElement; ++i)
    {
        std::cout << i <<" : " << typeNumberToString(lua_type(pState, -i)) << "\n";
    }
    std::cout << "----------------------\n";
}
