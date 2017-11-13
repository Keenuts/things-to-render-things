#pragma once

#include "types.hcl"
#include "helpers.hcl"

static
ray_t compute_ray(scene_t scene, int2 resolution, int2 pos)
{
  double width, height, L, t_width, t_height;
  double3 center, pt;

  width = resolution.x;
  height = resolution.y;

  L = width / (tan(DEG2RAD * (FOV * 0.5)) * 2.0);
  center = scene.camera_position + scene.camera_direction * L;

  t_width = mix(-width, width, pos.x / width) * 0.5;
  t_height= mix(-height, height, pos.y / height) * 0.5;

  pt = (double3)(1.0, 0.0, 0.0) * t_width
     + (double3)(0.0, -1.0, 0.0) * t_height
     + center;

  ray_t r;
  r.origin = scene.camera_position;
  r.direction = normalize(pt - scene.camera_position);

  return r;
}

static
bool intersect_sphere(ray_t r, double3 center, double rad, hit_t *hit)
{
    r.direction = normalize(r.direction);
    double3 e0 = center - r.origin;

    double v = dot(e0, r.direction);
    double d2 = dot(e0, e0) - v * v;
    double rad2 = rad * rad;

    if (d2 > rad2)
        return 0;

    double d = sqrt(rad2 - d2);

    double t0 = v - d;
    double t1 = v + d;

    if (t0 > t1)
        swap(&t0, &t1);

    if (t0 < 0)
        t0 = t1;

    if (t0 < 0)
        return 0;

    double3 p = r.origin + r.direction * t0;
    hit->position = p;
    hit->normal = normalize(p - center);

    return 1;
}

bool intersect_plane(ray_t r, double3 a, double3 normal, hit_t *hit)
{
    r.direction = normalize(r.direction);

    double d = dot(normal, r.direction);

    if (fabs(d) < 0.0001) //Ray // to tri
        return 0;

    double t = dot(a - r.origin, normal) / d;
    if (t < 0) //Tri behind our ray
        return 0;

    hit->position = r.origin + t * r.direction;
    hit->normal = normalize(normal);
    return true;
}

bool intersect_tri(ray_t r, double3 a, double3 b, double3 c, hit_t *out)
{
    hit_t hit;

    double3 ab = normalize(b - a);
    double3 ac = normalize(c - a);
    double3 normal = normalize(cross(ab, ac));

    if (dot(normal, r.direction) > 0) // Back culling
        return 0;

    if (!intersect_plane(r, a, normal, &hit))
        return 0;

    double3 tmp = cross(b - a, hit.position - a);
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
