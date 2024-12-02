#pragma once
#include "StaticMapElement.h"
#include <Texture.h>
#include <Vector2.h>

enum class BuildingType
{
    X4Y4,
    X5Y3,
    X5Y4,
};

class Building : public StaticMapElement
{
private:
    BuildingType m_buildingType; // x4y4 etc
    E2::Texture m_texture;
    E2::Vector2 m_position;
    E2::Vector2 m_dimension; 
    E2::Vector2 m_entrance; //unused
    
public:
    Building(BuildingType type, E2::Texture texture, E2::Vector2 Position, E2::Vector2 dimension);
    ~Building() = default;

    virtual void Draw() override;
    void SetPosition(E2::Vector2 pos) { m_position = pos; }
    void SetDimension(E2::Vector2 dimension) { m_dimension = dimension; }

    BuildingType GetBuildingType() const { return m_buildingType; }

};
