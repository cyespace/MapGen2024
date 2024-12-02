#pragma once
#include "HashString.h"

namespace E2
{
    namespace ComponentTypeDef
    {
        constexpr uint32_t kTransform = E2::Crc32("Transform");
        constexpr uint32_t kKinematic = E2::Crc32("Kinematic");
        constexpr uint32_t kImage = E2::Crc32("Image");
        constexpr uint32_t kShape = E2::Crc32("Shape");
        constexpr uint32_t kState = E2::Crc32("State");
    }
}