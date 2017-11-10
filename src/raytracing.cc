#include <math.h>
#include <stdint.h>

#include "raytracing.hh"

#if 1
uint8_t intersect_sphere(ray_t r, vec3_t center, double rad, vec3_t *out)
{
    r.direction = normalize(r.direction);
    vec3_t e0 = center - r.origin;

    double v = dot(e0, r.direction);
    double disc = rad * rad - (dot(e0, e0) - v * v);

    if (disc < 0)
        return 0;
    
    double d = sqrt(disc);
    vec3_t point_a = r.origin + r.direction * (v - d);
    double dot_pr = dot(-point_a, r.direction);

    vec3_t point_b = point_a + normalize(r.direction);
    point_b = point_b * rad * 2.0 * dot_pr;

    if (magnitude(r.origin - center) < rad)
        point_a = point_b;

    if (dot(point_a - r.origin, r.direction) <= 0.0f)
        return 0;

    *out = point_a;
    return 1;
}
#else

uint8_t intersect_sphere(ray_t r, vec3_t center, double rad, vec3_t *out)
{
    double t0, t1;
    double rad2 = rad * rad;
    vec3_t dir = normalize(r.direction);

    vec3_t l = center - r.origin;
    double tca = dot(l, dir);
    double d2 = dot(l, l) - tca * tca; 

    if (d2 > rad2)
        return 0;

    double tch = sqrt(rad2 - d2);
    t0 = tca - tch;
    t1 = tca + tch;

    if (t0 > t1)
        std::swap(t0, t1);

    if (t0 < 0) {
        t0 = t1;
        if (t0 < 0)
            return 0;
    }

    *out = r.origin + dir * t0;
    return 1;
}
#endif
