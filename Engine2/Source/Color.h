#pragma once
#include <cstdint>
#include <cmath>
namespace E2
{
    struct Color
    {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        uint8_t a = 255;
    };

    namespace MonoColor
    {
        const Color kBlack{ 0,0,0 };
        const Color kDark{ 50,50,50 };
        const Color kDarkGray{ 100,100,100 };
        const Color kGray{ 150,150,150 };
        const Color kLightGray{ 200,200,200 };
        const Color kWhite{ 255,255,255 };
    }

    namespace RedColor
    {
        const Color kLightPink{ 241,156,187 };
        const Color kSoftPink{ 255,105,180 };
        const Color kDarkPink{ 202,44,146 };
        const Color kCoral{ 248,131,121 };
        const Color kApricot{ 251,206,177 };
        const Color kDarkRed{ 139,0,0 };
        const Color kDarkPurple{ 102,2,60 };
        const Color kTuscan{ 102,66,77 };
        const Color kRedWood{ 91,52,46 };
        const Color kPink{ 255,0,255 };
        const Color kRed{ 255,0,0 };
    }
    
    namespace GreenColor
    {
        const Color kTeal{ 0,128,128 };
        const Color kTurquoise{ 64,224,208 };
        const Color kNeonGreen{ 57,255,20 };
        const Color kForest{ 34,139,34 };
        const Color kOlive{ 128,128,0 };
        const Color kLightGreen{ 144,238,144 };
        const Color kYellowGreen{ 154,205,50 };
        const Color kDarkGreen{ 33,66,30 };
        const Color kNightGreen{ 0,73,83 };
        const Color kCyan{ 0,255,255 };
        const Color kGreen{ 0,255,0 };
    }
    
    namespace BlueColor
    {
        const Color kLightBlue{ 173,216,230 };
        const Color kCobalt{ 0,71,171 };
        const Color kNavy{ 0,0,128 };
        const Color kRoyalBlue{ 65,105,225 };
        const Color kTiffanyBlue{ 10,186,181 };
        const Color kSkyBlue{ 0,191,255 };
        const Color kTrueBlue{ 0,115,207 };
        const Color kOxfordBlue{ 0,33,71 };
        const Color kCornflowerBlue{ 100,149,237 };
        const Color kBlue{ 0,0,255 };
    }
    
    namespace YellowColor
    {
        const Color kLemon{ 255,244,79 };
        const Color kGoldYellow{ 255,223,0 };
        const Color kBrassYellow{ 181,166,66 };
        const Color kMustard{ 225,173,1 };
        const Color kOchre{ 204,119,34 };
        const Color kDarkYellow{ 155,135,12 };
        const Color kOrangeYellow{ 255,174,66 };
        const Color kOrange{ 255,165,0 };
        const Color kJasmine{ 248,222,126 };
        const Color kYellowGray{ 194,178,128 };
        const Color kYellow{ 255,255,0 };
    }

    // copied from Josh's demo
    inline static Color Color_hsva(float hue, float saturation, float value, float alpha = 1.0f)
    {
        hue = fmodf(hue, 360.0f);
        float c = value * saturation;
        float x = c * (1 - fabsf(fmodf(hue / 60.0f, 2) - 1));
        float m = value - c;

        float r = m, g = m, b = m;

        if (0 <= hue && hue < 60) { r += c; g += x; b += 0; }
        else if (60 <= hue && hue < 120) { r += x; g += c; b += 0; }
        else if (120 <= hue && hue < 180) { r += 0; g += c; b += x; }
        else if (180 <= hue && hue < 240) { r += 0; g += x; b += c; }
        else if (240 <= hue && hue < 300) { r += x; g += 0; b += c; }
        else if (300 <= hue && hue < 360) { r += c; g += 0; b += x; }

        return {
            static_cast<uint8_t>(0xFF * r),
            static_cast<uint8_t>(0xFF * g),
            static_cast<uint8_t>(0xFF * b),
            static_cast<uint8_t>(0xFF * alpha)
        };
    }
}