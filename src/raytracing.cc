#include <math.h>
#include <stdint.h>

#include "raytracing.hh"

uint8_t intersect_sphere(ray_t r, vec3_t center, double rad, hit_t *hit)
{
    r.direction = normalize(r.direction);
    vec3_t e0 = center - r.origin;

    double v = dot(e0, r.direction);
    double d2 = dot(e0, e0) - v * v;
    double rad2 = rad * rad;

    if (d2 > rad2)
        return 0;

    double d = sqrt(rad2 - d2);

    double t0 = v - d;
    double t1 = v + d;

    if (t0 > t1)
        std::swap(t0, t1);

    if (t0 < 0)
        t0 = t1;

    if (t0 < 0)
        return 0;

    hit->position = r.origin + r.direction * t0;
    hit->normal = normalize(hit->position - center);

    return 1;
}

uint8_t intersect_plane(ray_t r, vec3_t a, vec3_t b, vec3_t c, hit_t *hit)
{
    r.direction = normalize(r.direction);

    vec3_t ab = normalize(b - a);
    vec3_t ac = normalize(c - a);

    vec3_t normal = normalize(cross(ab, ac));
    double d = dot(normal, r.direction);

    if (abs(d) < 0.0001) //Ray // to tri
        return 0;

    double t = dot(a - r.origin, normal) / d;
    if (t < 0) //Tri behind our ray
        return 0;

    hit->position = r.origin + t * r.direction;
    hit->normal = normalize(normal);
    return true;
}

uint8_t intersect_tri(ray_t r, vec3_t a, vec3_t b, vec3_t c, hit_t *out)
{
    hit_t hit;

    if (!intersect_plane(r, a, b, c, &hit))
        return 0;

    vec3_t tmp = cross(b - a, hit.position - a);
    if (dot(hit.normal, tmp) < 0)
        return 0;

    tmp = cross(c - b, hit.position - b);
    if (dot(hit.normal, tmp) < 0)
        return 0;

    tmp = cross(a - c, hit.position - c);
    if (dot(hit.normal, tmp) < 0)
        return 0;

    *out = hit;
    return 1;
}
