#pragma once
#include <cmath>
namespace E2
{
    struct Vector2
    {
        int x;
        int y;

        Vector2()
            :Vector2(0,0)
        {
        }

        Vector2(int inX, int inY)
            : x{inX}
            , y{inY}
        {
        }
    };

    struct Vector2f
    {
        float x;
        float y;

        Vector2f()
            :Vector2f(0, 0)
        {
        }

        Vector2f(float inX, float inY)
            : x{ inX }
            , y{ inY }
        {
        }

        //dot product
        static float Dot(const Vector2f& left, const Vector2f& right)
        {
            return left.x * right.x + left.y * right.y;
        }

        Vector2f operator*(float value) const
        {
            return Vector2f(x * value , y * value);
        }

        Vector2f operator/(float value) const
        {
            return Vector2f{ x / value, y / value };
        }

        Vector2f operator+(const Vector2f& left) const
        {
            return Vector2f{ x + left.x , y + left.y };
        }

        Vector2f operator-(const Vector2f& left) const
        {
            return Vector2f{ x - left.x , y - left.y };
        }

        void operator+=(const Vector2f& left)
        {
            x += left.x;
            y += left.y;
        }

        void operator-=(const Vector2f& left)
        {
            x -= left.x;
            y -= left.y;
        }

        void operator*=(const Vector2f& left)
        {
            x *= left.x;
            y *= left.y;
        }

        void operator*=(float value)
        {
            x *= value;
            y *= value;
        }

        void operator/=(float value)
        {
            x /= value;
            y /= value;
        }

        float Magnitude2() const 
        {
            return x * x + y * y;
        }

        float Magnitude() const
        {
            return std::sqrtf(Magnitude2());
        }

        Vector2f Front() const
        {
            float length = Magnitude();
            if (length == 0)
            {
                return Vector2f();
            }
            return Vector2f(x / length,y / length );
        }

        void Normalize()
        {
            float length = Magnitude();
            if (length == 0)
            {
                return;//what is the normal of 0 vector??
            }
            x /= length;
            y /= length;
        }

        void Clear()
        {
            x = 0;
            y = 0;
        }

        bool IsEqual(Vector2f& rhs,float tolerance)
        {
            if ((x - rhs.x) * (x - rhs.x) < tolerance * tolerance)
            {
                if ((y - rhs.y) * (y - rhs.y) < tolerance * tolerance)
                {
                    return true;
                }
            }
            return false;
        }
    };
}