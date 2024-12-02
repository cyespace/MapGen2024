#pragma once
#include "Macro.h"
#include "Texture.h"
#include "Font.h"
namespace E2
{
    struct GameSetting;
    struct Rect;
}

namespace E2
{
    class Device
    {
    private:
        Window* m_pWindow;
        Renderer* m_pRenderer;
    public:
        Device();
        ~Device();
        bool Init(const GameSetting& info);
        void ProcessInput();

        void DrawRect(int x,int y,int w,int h,uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void DrawRectOutline(int x,int y,int w,int h,uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void DrawLine(int x1, int y1, int x2, int y2,uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void DrawImageFromMem(std::byte* p, size_t size, Rect* pSrc, Rect* pDest);
        void DrawTexture(Texture& texture, Rect* pSrc, Rect* pDest);
        void DrawPoint(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void GetWindowSize(int& x,int& y);
        void Render();
        void ForceRender();
        void CleanRenderer();
        Texture ScreenCapture(int x, int y, int w, int h);

        bool PointInRect(int pX, int pY, int x,int y,int w,int h);

        Texture CreateTexture(std::byte* pResource, size_t size, TextureType type);
        Font CreateFont(std::byte* pResource, size_t size, int height);
        Texture CreateTextTexture(Font& font, std::string& text, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

        void DestroyTexture(E2::Texture& texture);
        void PlaySound();
        void ShutDown();
    };
}
