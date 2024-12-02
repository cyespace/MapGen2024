#pragma once
#include "Component.h"
#include "Vector2.h"
#include "Color.h"
#include <vector>

namespace E2
{
    class GameObject;
    class ShapeComponent : public Component
    {
    private:
        std::vector<Vector2> m_vertices;
        std::vector<Vector2> m_vertexPositions;
        std::vector<Vector2> m_edges;

        Color m_color = MonoColor::kWhite;

    public:
        ShapeComponent(GameObject* pOwner);
        virtual ~ShapeComponent();
        virtual void Update(float deltaTime) final;
        virtual void Draw() final;

        void AddVertex(int x, int y);
        void BuildEdge(int firstVertexId, int secondVertexId);
        void SetColor(E2::Color color) { m_color = color; }
    };
}