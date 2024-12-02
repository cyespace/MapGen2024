#include "MenuScene.h"
#include "MapGenerator.h"

#include <GlobalFunctions.h>
#include <HashString.h>
#include <UIElement.h>
#include <UIButton.h>
#include <UILabel.h>
#include <UITextInput.h>
#include <EventType.h>
#include <CallBack.h>

#include <iostream>

constexpr const char* kTitleScreen = "Script/UI/MenuScene.lua";
constexpr const char* kSeedButton = "Script/UI/MenuScene/Button_RandomSeed.lua";
constexpr const char* kGenerateMapButton = "Script/UI/MenuScene/Button_GenerateMap.lua";
constexpr const char* kQuitButton = "Script/UI/MenuScene/Button_Quit.lua";
void MenuScene::Update(float deltaTime)
{
}

bool MenuScene::Init()
{
    [[maybe_unused]]bool isUILoad = GetEngine().LoadUIElements(kTitleScreen);
    assert(isUILoad && "UI is not loaded.");

    //RandomSeed Button
    auto* pButton = GetEngine().Lua_LoadUIElement(kSeedButton);
    auto randomSeedButtonCallback = [](std::uintptr_t address)
    {
        auto seed = std::to_string(E2::Rand::Random());
        auto* pInputBox = GetEngine().GetUIElement("UITextInput");
        if (pInputBox)
        {
            auto* pTextInput = dynamic_cast<E2::UITextInput*>(pInputBox);
            pTextInput->ReplaceText(seed);
        }
    };
    dynamic_cast<E2::UIButton*>(pButton)->SetCallBack(new E2::CallBack(reinterpret_cast<std::uintptr_t>(this), randomSeedButtonCallback));


    //GenerateMap Button
    pButton = GetEngine().Lua_LoadUIElement(kGenerateMapButton);
    auto generateButtonCallback = [](std::uintptr_t address)
    {
        auto* pInputBox = GetEngine().GetUIElement("UITextInput");
        if (pInputBox)
        {
            auto* pTextInput = dynamic_cast<E2::UITextInput*>(pInputBox);
            MapGenerator::Get().SetMapSeed(E2::HashString(pTextInput->GetText()));
        }
        else
        {
            std::cout << "Cannot get seed. Using default seed. \n";
        }
        MapGenerator::Get().GenerateWorld();
        MapGenerator::Get().ViewWorld();
    };
    dynamic_cast<E2::UIButton*>(pButton)->SetCallBack(new E2::CallBack(reinterpret_cast<std::uintptr_t>(this), generateButtonCallback));


    //Quit Button
    pButton = GetEngine().Lua_LoadUIElement(kQuitButton);
    dynamic_cast<E2::UIButton*>(pButton)->SetCallBack([]() {GetEngine().Quit();});
    return true;
}

void MenuScene::End()
{
    GetEngine().ClearListener();
    GetEngine().ClearUI();
}
