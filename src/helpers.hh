#pragma once

#define D_EPSYLON 0.00001

template<typename T>
T lerp(T a, T b, T w)
{
    return a * (1.0 - w) + b * w;
}

template<typename T>
T clamp(T value, T down, T up)
{
    return value < down ? down : (value > up ? up : value);
}

template<typename T>
T is_zero(T v)
{
    return v < static_cast<T>(D_EPSYLON) && v > - static_cast<T>(D_EPSYLON);
}
