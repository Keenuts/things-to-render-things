#include <assert.h>
#include <math.h>
#include <string>

#include "vectors.hh"
#include "matrix.hh"
#include "helpers.hh"

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

vec3_t operator/(double a, vec3_t b)
{
    return b * (1.0 / a);
}

vec3_t operator/(vec3_t a, double b)
{
    return a * (1.0 / b);
}

vec3_t operator*(vec3_t a, vec3_t b)
{
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
    return a;
}

double dot(vec3_t a, vec3_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3_t cross(vec3_t a, vec3_t b)
{
    return vec3_t(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
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

vec3_t reflect(vec3_t i, vec3_t n)
{
    i = normalize(i);
    n = normalize(n);
    return normalize(i - 2.0 * n * dot(n, i));
}

vec3_t saturate(vec3_t c)
{
    c.r = clamp(c.r, 0.0, 1.0);
    c.g = clamp(c.g, 0.0, 1.0);
    c.b = clamp(c.b, 0.0, 1.0);
    return c;
}

double& vec3_t::operator[](int i)
{
    assert(i >= 0 && i < 3 && "Invalid subscript index on vector");

    if (i == 0)
        return x;
    if (i == 1)
        return y;
    return z;

}

void vec3_t::operator*=(double a)
{
    *this = *this * a;
}

void vec3_t::operator*=(vec3_t v)
{
    *this = *this * v;
}

void vec3_t::operator+=(vec3_t v)
{
    *this = *this + v;
}

void vec3_t::operator-=(vec3_t v)
{
    *this = *this - v;
}


vec3_t rotate(vec3_t in, vec3_t angles)
{
    mat3_t x_rot(0.0), y_rot(0.0), z_rot(0.0);

    x_rot[0] = vec3_t(1, 0,          0);
    x_rot[1] = vec3_t(0, cos(angles.x), -sin(angles.x));
    x_rot[2] = vec3_t(0, sin(angles.x), cos(angles.x));

    y_rot[0] = vec3_t(cos(angles.y),  0,          sin(angles.y));
    y_rot[1] = vec3_t(0,              1,          0);
    y_rot[2] = vec3_t(-sin(angles.y), 0,          cos(angles.y));

    z_rot[0] = vec3_t(cos(angles.z), -sin(angles.z), 0);
    z_rot[1] = vec3_t(sin(angles.z), cos(angles.z),  0);
    z_rot[2] = vec3_t(0,          0,           1);

    in = x_rot * (z_rot * (y_rot * in));
    return in;
}

static inline double rand_0_1()
{
    double n = (double)std::rand() / RAND_MAX;
    return n;
}

vec3_t get_sphere_random(void)
{
    double x, y, z;
    double m;

    do {
        x = 2.0 * rand_0_1() - 1.0;
        y = 2.0 * rand_0_1() - 1.0;
        z = 2.0 * rand_0_1() - 1.0;
        m = magnitude(vec3_t(x, y, z));
    } while (m > 1.0);

    return normalize(vec3_t(x, y, z));
}

vec3_t get_hemisphere_random(vec3_t dir)
{
    vec3_t d = get_sphere_random();
    if (dot(d, dir) < 0.0)
        d = -d;
    return d;
}
