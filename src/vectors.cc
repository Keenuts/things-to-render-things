#include <math.h>
#include <string>

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

vec3_t operator*(vec3_t a, double b)
{
    a.x *= b;
    a.y *= b;
    a.z *= b;
    return a;
}

vec3_t operator*(double a, vec3_t b)
{
    return b * a;
}

double dot(vec3_t a, vec3_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double magnitude(vec3_t v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3_t normalize(vec3_t v)
{
    double len = magnitude(v);
    if (len == 0.0f)
        return v;

    v.x /= len;
    v.y /= len;
    v.z /= len;

    return v;
}

std::string to_string(vec3_t v)
{
    return "vec3_t(" + std::to_string(v.x) + ";"
        + std::to_string(v.y) + ";"
        + std::to_string(v.z) + ")";
}
