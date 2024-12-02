#pragma once
#if defined(USING_SDL)

#include <SDL.h>
#include <SDL_ttf.h>
using Window = SDL_Window;
using Renderer = SDL_Renderer;
using _Texture = SDL_Texture;
using _Font = TTF_Font;
#elif defined(USING_SFML)

using Window = int;
using Renderer = int;
#endif
