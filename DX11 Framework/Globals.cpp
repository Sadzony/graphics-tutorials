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

int MathFunction::fib(int n) {
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}
