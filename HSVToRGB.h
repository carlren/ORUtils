#pragma once
#include "PlatformIndependence.h"
#include "Vector.h"
#include <math.h>

namespace SCFUSION {
    _CPU_AND_GPU_CODE_ inline void HSVtoRGB(float& fR, float& fG, float& fB, float& fH, float& fS, float& fV) {
        float fC = fV * fS; // Chroma
        float fHPrime = fmod(fH / 60.f, 6.f);
        float fX = fC * (1 - fabs(fmod(fHPrime, 2.f) - 1));
        float fM = fV - fC;

        if(0 <= fHPrime && fHPrime < 1) {
            fR = fC;
            fG = fX;
            fB = 0;
        } else if(1 <= fHPrime && fHPrime < 2) {
            fR = fX;
            fG = fC;
            fB = 0;
        } else if(2 <= fHPrime && fHPrime < 3) {
            fR = 0;
            fG = fC;
            fB = fX;
        } else if(3 <= fHPrime && fHPrime < 4) {
            fR = 0;
            fG = fX;
            fB = fC;
        } else if(4 <= fHPrime && fHPrime < 5) {
            fR = fX;
            fG = 0;
            fB = fC;
        } else if(5 <= fHPrime && fHPrime < 6) {
            fR = fC;
            fG = 0;
            fB = fX;
        } else {
            fR = 0;
            fG = 0;
            fB = 0;
        }

        fR += fM;
        fG += fM;
        fB += fM;
    }

    _CPU_AND_GPU_CODE_ inline ORUtils::Vector4<float> ValueHSVToRGBA(const float &value, const float &render_low_bound, const float &render_high_bound) {
        ORUtils::Vector4<float> RGBA;
        float H,s=1,v=1,r,g,b;
        if(value <= render_low_bound || value >= render_high_bound) {
            RGBA.x = 0;
            RGBA.y = 0;
            RGBA.z = 0;
            RGBA.w = 0;
            return RGBA;
        } else {
            H = 240 * (value - render_low_bound) /
                (render_high_bound - render_low_bound);
        }
        HSVtoRGB(r,g,b,H,s,v);
        RGBA.x = r;
        RGBA.y = g;
        RGBA.z = b;
        RGBA.w = 1;
        return RGBA;
    }

}