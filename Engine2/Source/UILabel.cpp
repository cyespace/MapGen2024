#include "pch.h"
#include "UILabel.h"
#include "Engine.h"

constexpr int kGlyphW = 49;
constexpr int kGlyphH = 46;
E2::UILabel::UILabel(Texture fontTexture, std::string& text)
    : m_fontTexture{ fontTexture }
    , m_originalText{ text }
    , m_isWordWrapping{ false }
    , m_horizontalAlign{ HorizontalAlign::Left }
    , m_verticalAlign{ VerticalAlign::Top }
    , m_useBitMap{true}
{
    if (text.size() > 0)
    {
        //DivideText();
    }
}

E2::UILabel::UILabel(Font font, std::string& text, Color textColor)
    : m_originalText{text}
    , m_isWordWrapping{ false }
    , m_horizontalAlign{ HorizontalAlign::Left }
    , m_verticalAlign{ VerticalAlign::Top }
    , m_useBitMap{ false }
    , m_font{font}
    , m_textColor{ textColor }
{
    if (text.size() != 0)
    {
        ReTexture();
    }
}

E2::UILabel::~UILabel()
{
}

void E2::UILabel::Draw()
{
    if (!m_isVisable)
        return;

    if (m_useBitMap)
    {
        Rect destRect{ 0,0,0,0 };
        destRect.w = kGlyphW;
        destRect.h = kGlyphH;

        // break down the original text into multiple lines
        std::vector<std::string> texts = DivideText();

        int lineCount = 0;
        // find the correct starting (x,y) for each line of the text
        for (auto line : texts)
        {
            int lineWidth = (int)line.size() * destRect.w;
            int spacing = 0;
            bool horizontalOverflow = (m_realRegion.w <= lineWidth);
            bool verticalOverflow = (m_realRegion.h <= texts.size() * destRect.h);

            switch (m_horizontalAlign)
            {
            case HorizontalAlign::Left: destRect.x = m_realRegion.x; break;
            case HorizontalAlign::Right:  destRect.x = horizontalOverflow ? m_realRegion.x : (m_realRegion.w - lineWidth); break;
            case HorizontalAlign::Center:  destRect.x = horizontalOverflow ? m_realRegion.x : (m_realRegion.w - lineWidth) / 2; break;
            case HorizontalAlign::Justified: destRect.x = m_realRegion.x; spacing = horizontalOverflow ? m_realRegion.x : ((m_realRegion.w - lineWidth) / ((int)line.size() - 1)); break; //
            }

            switch (m_verticalAlign)
            {
            case VerticalAlign::Top: destRect.y = m_realRegion.y + lineCount * destRect.h; break;
            case VerticalAlign::Middle: destRect.y = verticalOverflow ? m_realRegion.y : (m_realRegion.h - (int)texts.size() * destRect.h) / 2 + lineCount * destRect.h; break;
            case VerticalAlign::Bottom: destRect.y = verticalOverflow ? m_realRegion.y : m_realRegion.h - ((int)texts.size() - lineCount) * destRect.h; break;
            }

            // draw each line
            for (auto c : line)
            {
                Rect glyphRect = GetGlyphRect(c);
                E2::Engine::Get().DrawTexture(m_fontTexture, &glyphRect, &destRect);
                destRect.x += destRect.w + spacing;
                if (destRect.x > m_realRegion.w + m_realRegion.x)
                {
                    break;
                }
            }
            ++lineCount;
        }
    }
    else
    {
        E2::Rect rect{ m_realRegion.x, m_realRegion.y, m_stringTexture.dimension.x, m_stringTexture.dimension.y };
        E2::Engine::Get().DrawTexture(m_stringTexture, nullptr, &rect);
    }
}


void E2::UILabel::SetStyle(HorizontalAlign h, VerticalAlign v, bool wrap)
{
    m_horizontalAlign = h;
    m_verticalAlign = v;
    m_isWordWrapping = wrap;
}

void E2::UILabel::ChangeHorizontal()
{
    switch (m_horizontalAlign)
    {
    case HorizontalAlign::Left: m_horizontalAlign = HorizontalAlign::Center; break;
    case HorizontalAlign::Center: m_horizontalAlign = HorizontalAlign::Right; break;
    case HorizontalAlign::Right: m_horizontalAlign = HorizontalAlign::Justified; break;
    case HorizontalAlign::Justified: m_horizontalAlign = HorizontalAlign::Left; break;
    }
}

void E2::UILabel::ChangeVertical()
{
    switch (m_verticalAlign)
    {
    case VerticalAlign::Top: m_verticalAlign = VerticalAlign::Middle; break;
    case VerticalAlign::Middle: m_verticalAlign = VerticalAlign::Bottom; break;
    case VerticalAlign::Bottom: m_verticalAlign = VerticalAlign::Top; break;
    }
}

void E2::UILabel::WordWrap()
{
    m_isWordWrapping = !m_isWordWrapping;
}

void E2::UILabel::SetText(const std::string& newText)
{
    m_originalText = newText;
    if (!m_useBitMap)
    {
        ReTexture();
    }
}

E2::Rect E2::UILabel::GetGlyphRect(char c)
{
    constexpr int kColumns = 10;

    Rect glyph;
    glyph.w = kGlyphW;
    glyph.h = kGlyphH;

    if (c >= 'A' && c <= 'Z')
    {
        glyph.x = (c - 'A') % kColumns;
        glyph.y = (c - 'A') / kColumns;
    }
    else if (c >= 'a' && c <= 'z')
    {
        int index = 26 + (c - 'a');
        glyph.x = index % kColumns;
        glyph.y = index / kColumns;
    }
    else if (c >= '0' && c <= '9')
    {
        int index = 52 + (c - '0');
        glyph.x = index % kColumns;
        glyph.y = index / kColumns;
    }
    else
    {
        int index = 90;
        glyph.x = index % kColumns;
        glyph.y = index / kColumns;
    }

    glyph.x *= glyph.w;
    glyph.y *= glyph.h;

    return glyph;
}

std::vector<std::string> E2::UILabel::DivideText()
{
    std::vector<std::string> texts;
    size_t start = 0;
    size_t end = 0;
    for (size_t i = 0; i < m_originalText.size(); ++i)
    {
        if (m_originalText[i] == '\n')
        {
            end = i;
            auto substring = m_originalText.substr(start, end - start);
            texts.emplace_back(substring);
            start = i + 1;
            continue;
        }
        

        if (m_isWordWrapping)
        {
            int count = (int)(i - start + 1);
            int width = count * kGlyphW;
            if (width > m_realRegion.w)
            {
                end = i;
                auto substring = m_originalText.substr(start, end - start);
                texts.emplace_back(substring);
                start = end;
            }
        }

        if (i == m_originalText.size() - 1)
        {
            auto substring = m_originalText.substr(start, m_originalText.size() - start);
            texts.emplace_back(substring);
            continue;
        }
    }
    return texts;
}

void E2::UILabel::ReTexture()
{
    m_stringTexture = E2::Engine::Get().CreateTextTexture(m_font, m_originalText, m_textColor);
}
