#include "Globals.h"

float MathFunction::lerp(float a, float b, float t)
{
    if (t >= 0 && t <= 1) {
        return a + t * (b - a);
    }
    else if (t > 1) {
        return a + (b - a);
    }
    else if (t < 0) {
        return a;
    }
}
