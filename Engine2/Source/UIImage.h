#pragma once
#include "UIElement.h"
#include "Texture.h"
#include "Rect.h"

#include <vector>

namespace E2
{
    struct UIImageData : UIElementData
    {
        std::string texture;
        Rect srcRect{};
        Rect destRect{};
        bool isSliced = false;
        float leftSlice = 0;
        float rightSlice = 0;
        float topSlice = 0;
        float bottomSlice = 0;
    };

    class UIImage : public UIElement
    {
    private:
        Texture m_texture;
        bool m_isSliced;
        UIDesignCoord m_leftSlice;
        UIDesignCoord m_rightSlice;
        UIDesignCoord m_topSlice;
        UIDesignCoord m_downSlice;
        std::vector<Rect> m_destRects;
        std::vector<Rect> m_sourceRects;
        
    public:
        UIImage(Texture texture, Rect src, Rect dest, UIDesignCoord x, UIDesignCoord y
            , UIDesignCoord w, UIDesignCoord h, UIDesignCoord x0, UIDesignCoord y0);
            
        virtual ~UIImage();
        virtual void Draw();

        void SetImage(Texture texture, E2::Rect newSource = E2::Rect{});
        void SetSlice(UIDesignCoord left, UIDesignCoord right, UIDesignCoord top, UIDesignCoord down);
    };
}