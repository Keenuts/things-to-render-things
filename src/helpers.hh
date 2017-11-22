#pragma once

#define D_EPSYLON 0.00001
#define F_EPSYLON 0.001f
#define PI 3.14159265358979323846
#define DEG2RAD(x) ((x) * ((PI * 2.0) / 360.0))

template<typename T>
T lerp(T a, T b, float w)
{
    return a * (1.0f - w) + b * w;
}

template<typename T>
T clamp(T value, T down, T up)
{
    return value < down ? down : (value > up ? up : value);
}

template<typename T>
T is_zero(T v)
{
    return v < static_cast<T>(F_EPSYLON) && v > - static_cast<T>(D_EPSYLON);
}
