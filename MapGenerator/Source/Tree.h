#pragma once
#include "StaticMapElement.h"
#include <Texture.h>
#include <Vector2.h>
#include <Rect.h>

enum class TreeType
{
    Cold,
    Green,
    Yellow,
    Rain,
    HalfSnow,
    Dead,
    None,
};

class Tree : public StaticMapElement
{
private:
    E2::Texture m_texture;
    E2::Texture m_isometricTexture;
    E2::Vector2 m_position;
    E2::Vector2 m_dimension;
    E2::Rect m_rect;
    E2::Rect m_isometricRect;
public:
    Tree(E2::Texture texture, E2::Vector2 pos, E2::Vector2 dimension);
    ~Tree() = default;
    virtual void Draw()override;
    void CreateIsometric(E2::Texture texture, E2::Rect rect);
    void DrawIsometric();
};