#include "Tree.h"


#include <FreeFunctions.h>


Tree::Tree(E2::Texture texture, E2::Vector2 pos, E2::Vector2 dimension)
    : m_texture{texture}
    , m_position{pos}
    , m_dimension{dimension}
{
    m_rect = { m_position.x ,m_position.y, m_dimension.x, m_dimension.y };
}

void Tree::Draw()
{
    DrawTexture(m_texture, nullptr, &m_rect);
}

void Tree::CreateIsometric(E2::Texture texture, E2::Rect rect)
{
    m_isometricTexture = texture;
    m_isometricRect = rect;
}

void Tree::DrawIsometric()
{
    DrawTexture(m_isometricTexture, nullptr, &m_isometricRect);
}
