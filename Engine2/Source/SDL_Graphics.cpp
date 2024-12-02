#include "pch.h"

#include "Device.h"
#include "Rect.h"
#include "Engine.h"
#include "HashString.h"

#include <SDL_image.h>
#include <iostream>

static std::unordered_map<size_t, E2::Texture> s_TextureMap;

void E2::Device::DrawRect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    SDL_SetRenderDrawColor(m_pRenderer, r, g, b, a);
    SDL_Rect rect{ x,y,w,h };
    SDL_RenderFillRect(m_pRenderer, &rect);
}

void E2::Device::DrawRectOutline(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    SDL_SetRenderDrawColor(m_pRenderer, r, g, b, a);
    SDL_Rect rect{ x,y,w,h };
    SDL_RenderDrawRect(m_pRenderer, &rect);
}

void E2::Device::DrawLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    SDL_SetRenderDrawColor(m_pRenderer, r, g, b, a);
    SDL_RenderDrawLine(m_pRenderer, x1, y1, x2, y2);
}

void E2::Device::DrawImageFromMem(std::byte* p, size_t size, Rect* pSrc, Rect* pDest)
{
    SDL_RWops* pRWop = SDL_RWFromMem(p, (int)size);
    SDL_Surface* pSurface = SDL_LoadBMP_RW(pRWop,0);
    SDL_Texture* pTexture = SDL_CreateTextureFromSurface(m_pRenderer, pSurface);
    SDL_Rect source;
    SDL_Rect destination;
    SDL_Rect* pSource = nullptr;
    SDL_Rect* pDestination = nullptr;
    if (pSrc)
    {
        source = { pSrc->x,pSrc->y,pSrc->w,pSrc->h };
        pSource = &source;
    }
    if (pDest)
    {
        destination = { pDest->x,pDest->y,pDest->w,pDest->h };
        pDestination = &destination;
    }
    auto error = SDL_RenderCopy(m_pRenderer, pTexture, pSource, pDestination);
    if (error == -1)
    {
        std::cout << SDL_GetError() << '\n';
        assert(false&&"Render Error");

    }
    SDL_DestroyTexture(pTexture);
    pTexture = nullptr;
    SDL_FreeSurface(pSurface);
    pSurface = nullptr;
    SDL_RWclose(pRWop);
    pSurface = nullptr;
}

void E2::Device::DrawTexture(Texture& texture, Rect* pSrc, Rect* pDest)
{
    SDL_Rect source;
    SDL_Rect destination;
    SDL_Rect* pSource = nullptr;
    SDL_Rect* pDestination = nullptr;
    if (pSrc)
    {
        source = { pSrc->x,pSrc->y,pSrc->w,pSrc->h };
        pSource = &source;
    }
    if (pDest)
    {
        destination = { pDest->x,pDest->y,pDest->w,pDest->h };
        pDestination = &destination;
    }
    SDL_RenderCopy(m_pRenderer, texture.pTexture, pSource, pDestination);
}

void E2::Device::DrawPoint(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    SDL_SetRenderDrawColor(m_pRenderer, r, g, b, a);
    SDL_RenderDrawPoint(m_pRenderer, x, y);
}

void E2::Device::GetWindowSize(int& x, int& y)
{
    SDL_GetWindowSize(m_pWindow,&x,&y);
}

void E2::Device::Render()
{
    SDL_RenderPresent(m_pRenderer);
    SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255);
    SDL_RenderClear(m_pRenderer);
}

void E2::Device::ForceRender()
{
    SDL_RenderPresent(m_pRenderer);
}

void E2::Device::CleanRenderer()
{
    SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255);
    SDL_RenderClear(m_pRenderer);
}

E2::Texture E2::Device::ScreenCapture(int x, int y, int w, int h)
{
    SDL_Rect rect{ x,y,w,h };

    if (x < 0 || y < 0 || w < 0 || h < 0)
    {
        //default take full screen shot
        int windowWidth = 0;
        int windowHeight = 0;
        SDL_GetWindowSize(m_pWindow, &windowWidth, &windowHeight);
        rect = {0,0,windowWidth,windowHeight};
    }
    else
    {
        rect = { x,y,w,h };
    }


    E2::Texture currentView{};
    SDL_Surface* pScreenCap = SDL_CreateRGBSurface(0, rect.w, rect.h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    SDL_RenderReadPixels(m_pRenderer, &rect, SDL_PIXELFORMAT_ARGB8888, pScreenCap->pixels, pScreenCap->pitch);
    SDL_Texture* pScreenCapTexture = SDL_CreateTextureFromSurface(m_pRenderer, pScreenCap);

    currentView.dimension.x = rect.w;
    currentView.dimension.y = rect.h;
    currentView.pTexture = pScreenCapTexture;
    currentView.type = TextureType::Unknown;

    SDL_FreeSurface(pScreenCap);
    return currentView;
}

E2::Texture E2::Device::CreateTexture(std::byte* pResource, size_t size, TextureType type)
{
    Texture texture{};
    if (pResource)
    {
        SDL_RWops* pRWop = SDL_RWFromMem(pResource, (int)size);
        SDL_Surface* pSurface = nullptr;
        if (type == TextureType::BMP)
        {
            pSurface = SDL_LoadBMP_RW(pRWop, 0);
        }
        else if (type == TextureType::PNG)
        {
            pSurface = IMG_LoadPNG_RW(pRWop);
        }
        else
        {
            //unsupported format
            assert(false && "Unsupported Image Format");
        }
        
        SDL_Texture* pTexture = SDL_CreateTextureFromSurface(m_pRenderer, pSurface);
        texture.pTexture = pTexture;
        texture.type = type;    //TODO: change type by file extension
        if (pSurface)
        {
            texture.dimension = { pSurface->w, pSurface->h };
        }
        SDL_FreeSurface(pSurface);
        SDL_RWclose(pRWop);
    }

    return texture;
}

E2::Font E2::Device::CreateFont(std::byte* pResource, size_t size, int height)
{
    Font font;
    if (pResource)
    {
        SDL_RWops* pRWop = SDL_RWFromMem(pResource, (int)size);
        font.pFont = TTF_OpenFontRW(pRWop, 0, height);
        font.fontSize = height;
        //SDL_RWclose(pRWop); TODO: Why if do this font pointer will go bad

        //test
        //auto* pSurface = TTF_RenderText_Solid(font.pFont,"hello", { 255, 255, 255, 255 });
        //SDL_Texture* pTexture = SDL_CreateTextureFromSurface(m_pRenderer, pSurface);
        //SDL_RenderCopy(m_pRenderer, pTexture, NULL, NULL);
    }

    return font;
}

E2::Texture E2::Device::CreateTextTexture(Font& font, std::string& text, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    Texture texture{};
    if (font.pFont && text.size()!=0)
    {
        auto* pSurface = TTF_RenderText_Solid(font.pFont, text.c_str(), {r, g, b, a });
        SDL_Texture* pTexture = SDL_CreateTextureFromSurface(m_pRenderer, pSurface);
        texture.pTexture = pTexture;
        texture.type = TextureType::Text;    //TODO: change type 
        texture.dimension = { pSurface->w, pSurface->h };
        font.charW = pSurface->w / (int)text.size();
        font.charH = pSurface->h;
        SDL_FreeSurface(pSurface);
    }

    return texture;
}

void E2::Device::DestroyTexture(E2::Texture& texture)
{
    SDL_DestroyTexture(texture.pTexture);
}