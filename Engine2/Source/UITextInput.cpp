#include "pch.h"
#include "UITextInput.h"
#include "UILabel.h"
#include "UIRect.h"
#include "GlobalFunctions.h"

E2::UITextInput::UITextInput(Texture texture, Color& backGroundcolor)
{
    m_pBackground = new UIRect(backGroundcolor,false,true);
    m_pBackground->SetDimension({ UICoordType::Percentage, 1 }, { UICoordType::Percentage, 1 });

    std::string str{};
    m_pContents = new UILabel(texture, str);
    m_pContents->SetDimension({ UICoordType::Percentage, 1 }, { UICoordType::Percentage, 1 });
    m_pContents->WordWrap();
    AddChild(m_pBackground);
    AddChild(m_pContents);
}

E2::UITextInput::UITextInput(Font font, Color textColor, Color backGroundcolor)
{
    m_pBackground = new UIRect(backGroundcolor, false, true);
    m_pBackground->SetDimension({ UICoordType::Percentage, 1 }, { UICoordType::Percentage, 1 });

    std::string str{"A"};
    m_pContents = new UILabel(font,str,textColor);
    m_pContents->SetDimension({ UICoordType::Percentage, 1 }, { UICoordType::Percentage, 1 });
    m_charW = m_pContents->GetFont().charW;
    m_charH = m_pContents->GetFont().charH;

    std::string empty{ "" };
    m_pContents->SetText(empty);
    AddChild(m_pBackground);
    AddChild(m_pContents);
}

E2::UITextInput::~UITextInput()
{
}

void E2::UITextInput::OnKeyDown(E2::Keyboard::Key key)
{
    if (key == E2::Keyboard::Key::BackSpace && m_cursorIndex != 0 && !m_text.empty())
    {
        --m_cursorIndex;
        m_text.erase(m_cursorIndex, 1);
        m_pContents->SetText(m_text);
    }
    else if (key == E2::Keyboard::Key::Left)
    {
        if (m_cursorIndex > 0)
            --m_cursorIndex;
    }
    else if (key == E2::Keyboard::Key::Right)
    {
        if (m_cursorIndex < m_text.size())
            ++m_cursorIndex;
    }
}

void E2::UITextInput::OnText(const char c)
{
    if (!m_isFocused)
        return;
    //TODO: Make Proper scrolling
    if (m_cursorIndex >= m_maxCharPerLine * m_maxLine)
    {
        return;
    }

    m_text.insert(m_cursorIndex, 1, c);
    ++m_cursorIndex;
    m_pContents->SetText(m_text);
}

void E2::UITextInput::Draw()
{
    UIElement::Draw();
    if (!m_isVisable)
        return;

    if (m_isFocused)
    {
        m_maxCharPerLine = m_realRegion.w / m_charW;
        m_maxLine = m_realRegion.h / m_charH;
        // Draw the cursor
        int cursorX = 0;
        int cursorY = 0;
        if (m_cursorIndex / m_maxCharPerLine >= m_maxLine)
        {
            cursorX = m_realRegion.x + m_maxCharPerLine * m_charW;
            cursorY = m_realRegion.y + (m_maxLine -1) * m_charH;
        }
        else
        {
            cursorX = m_realRegion.x + (m_cursorIndex % m_maxCharPerLine) * m_charW;
            cursorY = m_realRegion.y + (m_cursorIndex / m_maxCharPerLine) * m_charH;
        }
        Engine::Get().DrawLine({ cursorX ,cursorY}, { cursorX ,cursorY + m_charH }, {255,255,255,255});
    }
}

void E2::UITextInput::OnNotify(Event evt)
{
    if (!m_isFocused)
        return;
    OnText(evt.m_keyBoardEvent.key);
}

void E2::UITextInput::ReplaceText(const std::string& text)
{
    m_text = text;
    m_cursorIndex = (int)m_text.size();
    m_pContents->SetText(text);
}
