#pragma once
#include "StaticMapElement.h"
#include <Texture.h>
#include <Vector2.h>
#include <Rect.h>

enum class GrassType
{
    Cold,
    Green,
    Rain,
    HalfSnow,
    Yellow,
    Tall,
    None,
};
class Grass : public StaticMapElement
{
private:
    E2::Texture m_texture;
    E2::Texture m_isometricTexture;
    E2::Vector2 m_position;
    E2::Vector2 m_dimension;
    E2::Rect m_rect;
    E2::Rect m_isometricRect;
public:
    Grass(E2::Texture texture, E2::Vector2 pos, E2::Vector2 dimension);
    ~Grass() = default;
    virtual void Draw()override;
    void DrawIsometric();
    void CreateIsometric(E2::Texture texture, E2::Rect rect);
};


