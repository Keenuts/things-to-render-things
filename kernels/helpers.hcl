#pragma once

#define GEN_SWAP(Type)        \
static inline                 \
void swap(Type *a, Type *b) { \
  Type c = *a;                \
  *a = *b;                    \
  *b = c;                     \
}

GEN_SWAP(double);

#define FOV 45.0
#define PI 3.14159265358979323846
#define DEG2RAD 0.017453292519943295
#define D_EPSYLON 0.00001
#define RAND_MAX 2147483647

static double3 saturate(double3 f)
{
    f.x = clamp(f.x, 0.0, 1.0);
    f.y = clamp(f.y, 0.0, 1.0);
    f.z = clamp(f.z, 0.0, 1.0);
    return f;
}

static double3 mult(double3 m[3], double3 v)
{
    double3 res;
    res.x = m[0].x * v.x + m[0].y * v.y + m[0].z * v.z;
    res.y = m[1].x * v.x + m[1].y * v.y + m[1].z * v.z;
    res.z = m[2].x * v.x + m[2].y * v.y + m[2].z * v.z;
    return res;
}

static double3 rotation(double3 in, double3 angles)
{
    double3 x_rot[3];
    double3 y_rot[3];
    double3 z_rot[3];

    x_rot[0] = (double3)(1, 0,          0);
    x_rot[1] = (double3)(0, cos(angles.x), -sin(angles.x));
    x_rot[2] = (double3)(0, sin(angles.x), cos(angles.x));

    y_rot[0] = (double3)(cos(angles.y),  0,          sin(angles.y));
    y_rot[1] = (double3)(0,              1,          0);
    y_rot[2] = (double3)(-sin(angles.y), 0,          cos(angles.y));

    z_rot[0] = (double3)(cos(angles.z), -sin(angles.z), 0);
    z_rot[1] = (double3)(sin(angles.z), cos(angles.z),  0);
    z_rot[2] = (double3)(0,          0,           1);

    in = mult(x_rot, mult(z_rot, mult(y_rot, in)));
    return in;
}

static int rand(scene_t *scene)
{
    const int a = 16807;
    const int m = RAND_MAX;

    int seed = scene->last_seed;
    seed = ((long)(seed * a)) % m;
    scene->last_seed = seed;
    return seed;
}

static double rand_0_1(scene_t *scene)
{
    return (double)rand(scene) / (double)RAND_MAX;
}

double3 get_hemisphere_random(scene_t *scene, double3 dir)
{
    double x, y, z;
    double m;

    do {
        x = 2.0 * rand_0_1(scene) - 1.0;
        y = 2.0 * rand_0_1(scene) - 1.0;
        z = 2.0 * rand_0_1(scene) - 1.0;
        m = length((double3)(x, y, z));
    } while (m > 1.0);

    double3 d = normalize((double3)(x, y, z));
    if (dot(d, dir) < 0.0)
        d = -d;

    return d;
}
