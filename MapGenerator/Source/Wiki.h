#pragma once
#include "MacroDef.h"

#include <Texture.h>

#include <string>
#include <vector>

namespace E2
{
    class UIElement;
}
class Wiki
{
public:
    struct CritterEntry
    {
        bool m_isFound = false;
        int m_id = BAD;
        std::string m_name = {};
        std::string m_description = {};
        E2::Texture m_texture = {};
    };

private:
    std::vector<CritterEntry> m_wiki;
    int m_currentId = BAD;
    bool m_isOpen = false;
    E2::UIElement* m_pBackGround = nullptr;
    E2::UIElement* m_pCritterImage = nullptr;
    E2::UIElement* m_pCritterId = nullptr;
    E2::UIElement* m_pCritterName = nullptr;
    E2::UIElement* m_pCritterDescription = nullptr;
public:
    Wiki() = default;
    ~Wiki() = default;

    bool Init(); //load entries from script
    void Open();
    void Close();

    void DiscoverCritter(int critterId);
    bool IsOpen() const { return m_isOpen; }
private:
    bool LoadData(const char* pWikiData);
    bool InitUI();
};