#include "Building.h"
#include <FreeFunctions.h>
#include <Rect.h>

Building::Building(BuildingType type, E2::Texture texture, E2::Vector2 Position, E2::Vector2 dimension)
    : m_buildingType{type}
    , m_texture {texture}
    , m_position{ Position }
    , m_dimension {dimension}
{
}

void Building::Draw()
{
    E2::Rect temp;
    temp.x = m_position.x;
    temp.y = m_position.y;
    temp.w = m_dimension.x;
    temp.h = m_dimension.y;
    DrawTexture(m_texture, nullptr, &temp);
}
