#pragma once
#include "UIElement.h"
#include "Rect.h"
#include "Texture.h"
#include "Font.h"
#include "Color.h"

namespace E2
{
    struct UILabelData : UIElementData
    {
        std::string content;
        std::string font;
        int fontSize = 0;
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
    };

    class UILabel : public UIElement
    {
    public:
        enum class HorizontalAlign
        {
            Left,
            Center,
            Right,
            Justified,
        };

        enum class VerticalAlign
        {
            Top,
            Middle,
            Bottom,
        };

    private:
        Texture m_fontTexture;
        std::string m_originalText;
        bool m_isWordWrapping;
        HorizontalAlign m_horizontalAlign;
        VerticalAlign m_verticalAlign;

        bool m_useBitMap;
        Font m_font;
        Color m_textColor;
        Texture m_stringTexture;
    public:
        UILabel(Texture fontTexture, std::string& text);
        UILabel(Font font, std::string& text, Color textColor);
        ~UILabel();
        virtual void Update() final {}
        virtual void Draw() final;
        void SetStyle(HorizontalAlign h, VerticalAlign v, bool wrap);

        void ChangeHorizontal();
        void ChangeVertical();
        void WordWrap();
        void SetText(const std::string& newText);
        const Font& GetFont() const { return m_font; }

    private:
        Rect GetGlyphRect(char c);
        std::vector<std::string> DivideText();
        void ReTexture();
    };
}