#pragma once

#include <Vector2.h>

#include <vector>

class Player
{
private:
    bool m_isAlive = false;
    E2::Vector2 m_position;
    std::vector<bool> m_critterWiki = std::vector<bool>(10);

public:

    void Move(int deltaX, int deltaY) { m_position.x += deltaX; m_position.y += deltaY; }
    void SetPosition(int x, int y) { m_position.x = x; m_position.y = y;}
    E2::Vector2 GetPosition() const { return m_position; }
    void SetAlive(bool isAlive) { m_isAlive = isAlive; }
    bool IsAlive()const { return m_isAlive; }
    void CatchCritter(int id) { m_critterWiki[id] = true; }
};