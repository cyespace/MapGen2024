#pragma once
#include <Rect.h>

class WorldMap;
class Camera
{
private:
    E2::Rect m_window;
    int m_xLimit = -1;
    int m_yLimit = -1;

public:
    Camera();
    ~Camera();
    void SetPosition(int x, int y);
    void SetSize(int w, int h);
    void SetLimit(int maxX, int maxY);
    void Move(int x, int y);
    E2::Rect GetWindow() const { return m_window; }
    void SetCenter(int x, int y);
};