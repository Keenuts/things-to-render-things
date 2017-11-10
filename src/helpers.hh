#pragma once

template<typename T>
T lerp(T a, T b, T w)
{
    return a * (1.0 - w) + b * w;
}
