#include "Camera.h"

#include <GlobalFunctions.h>

//TODO:
Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::SetPosition(int x, int y)
{
    if (x >= 0 && x <= m_xLimit - m_window.w)
    {
        m_window.x = x;
    }
    if (y >= 0 && y <= m_yLimit - m_window.h)
    {
        m_window.y = y;
    }
}

void Camera::Move(int x, int y)
{
    if (x + m_window.x >= 0 && x + m_window.x <= m_xLimit - m_window.w)
    {
        m_window.x += x;
    }
    if (y + m_window.y >= 0 && y + m_window.y <= m_yLimit - m_window.h)
    {
        m_window.y += y;
    }
}

void Camera::SetCenter(int x, int y)
{
    int newX = x - m_window.w / 2;
    int newY = y - m_window.h / 2;
    SetPosition(newX, newY);
}

void Camera::SetSize(int w, int h)
{
    m_window.w = w;
    m_window.h = h;
}

void Camera::SetLimit(int maxX, int maxY)
{
    m_xLimit = maxX;
    m_yLimit = maxY;
}
