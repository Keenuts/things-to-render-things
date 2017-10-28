#include <math.h>

#include "vectors.hh"

vec3_t operator+(vec3_t a, vec3_t b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

vec3_t operator-(vec3_t a)
{
    a.x = -a.x;
    a.y = -a.y;
    a.z = -a.z;
    return a;
}

vec3_t operator-(vec3_t a, vec3_t b)
{
    return a + (-b);
}

vec3_t operator*(vec3_t a, float b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
    return a;
}

float dot(vec3_t a, vec3_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float magnitude(vec3_t v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3_t normalize(vec3_t v)
{
    float len = magnitude(v);
    if (len == 0.0f)
        return 0;

    v.x /= len;
    v.y /= len;
    v.z /= len;

    return v;
}

