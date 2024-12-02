#pragma once
#include "Vector2.h"
namespace E2
{
    class GameObject;
    struct SteeringOutput
    {
        Vector2f linear = {};
        float angular = 0;

        SteeringOutput(const Vector2f inLinear, float inAngular)
            : linear{inLinear}
            , angular{inAngular}
        {}

        SteeringOutput():SteeringOutput(Vector2f(),0) {}

        SteeringOutput operator+ (const SteeringOutput& left) const
        {
            return SteeringOutput(linear + left.linear, angular + left.angular);
        }
    };

    class Steering
    {
    public:
        Steering() = default;
        virtual ~Steering() = default;
        virtual SteeringOutput GetSteering(GameObject* pA, GameObject* pB) = 0;
    };

    
}
