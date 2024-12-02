#include "pch.h"
#include "ShapeComponent.h"
#include "TransformComponent.h"
#include "GameObject.h"
#include "Engine.h"
#include "Color.h"

#include <cmath>
#include <iostream>

E2::ShapeComponent::ShapeComponent(GameObject* pOwner)
    : Component{pOwner}
{
}

E2::ShapeComponent::~ShapeComponent()
{

}

void E2::ShapeComponent::Update(float deltaTime)
{
    //TODO: FIX
    TransformComponent* pTransform = nullptr;
    float rot = pTransform->GetRotation();
    auto pos = pTransform->GetCenter();

    auto halfW = pTransform->GetDimension().x/2;
    auto halfH = pTransform->GetDimension().y/2;
    //rotate each vertex and move to object position
    for (int i = 0; i < m_vertices.size(); ++i)
    {
        auto newX = m_vertices[i].x - halfW;
        auto newY = m_vertices[i].y - halfH;
        auto rotX = -newX * std::cos(rot) + newY * std::sin(rot);
        auto rotY = newX * std::sin(rot) + newY * std::cos(rot);

        m_vertexPositions[i].x = (int)rotX + (int)pos.x;
        m_vertexPositions[i].y = (int)rotY + (int)pos.y;
    }
}

void E2::ShapeComponent::Draw()
{
    for (auto& edge : m_edges)
    {
        E2::Engine::Get().DrawLine(m_vertexPositions[edge.x], m_vertexPositions[edge.y], m_color);
    }
}

void E2::ShapeComponent::AddVertex(int x, int y)
{
    m_vertices.emplace_back( x,y );
    m_vertexPositions.emplace_back( x,y );
}

void E2::ShapeComponent::BuildEdge(int firstVertexId, int secondVertexId)
{
    m_edges.emplace_back(firstVertexId, secondVertexId);
}
