#pragma once

static
ray_t compute_ray(scene_t scene, int2 resolution, int2 pos)
{
  float width, height, L, t_width, t_height;
  float3 center, pt;

  width = resolution.x;
  height = resolution.y;

  L = width / (tan(DEG2RAD * (FOV * 0.5f)) * 2.0f);
  center = scene.camera_position + scene.camera_direction * L;

  t_width = mix(-width, width, pos.x / width) * 0.5f;
  t_height= mix(-height, height, pos.y / height) * 0.5f;

  pt = (float3)(1.0f, 0.0f, 0.0f) * t_width
     + (float3)(0.0f, -1.0f, 0.0f) * t_height
     + center;

  ray_t r;
  r.origin = scene.camera_position;
  r.direction = normalize(pt - scene.camera_position);

  return r;
}

static
bool intersect_sphere(ray_t r, float3 center, float rad, hit_t *hit)
{
    r.direction = normalize(r.direction);
    float3 e0 = center - r.origin;

    float v = dot(e0, r.direction);
    float d2 = dot(e0, e0) - v * v;
    float rad2 = rad * rad;

    if (d2 > rad2)
        return false;

    float d = sqrt(rad2 - d2);

    float t0 = v - d;
    float t1 = v + d;

    if (t0 > t1)
        swap(&t0, &t1);

    if (t0 < 0)
        t0 = t1;

    if (t0 < 0)
        return false;

    float3 p = r.origin + r.direction * t0;
    hit->position = p;
    hit->normal = normalize(p - center);

    return true;
}

bool intersect_plane(ray_t r, float3 a, float3 normal, hit_t *hit)
{
    r.direction = normalize(r.direction);

    float d = dot(normal, r.direction);

    if (fabs(d) < F_EPSYLON) //Ray // to tri
        return false;

    float t = dot(a - r.origin, normal) / d;
    if (t < 0) //Tri behind our ray
        return false;

    hit->position = r.origin + t * r.direction;
    hit->normal = normalize(normal);
    return true;
}

bool intersect_tri(ray_t r, float3 a, float3 b, float3 c, hit_t *out)
{
    hit_t hit;

    float3 ab = normalize(b - a);
    float3 ac = normalize(c - a);
    float3 normal = normalize(cross(ab, ac));

    if (dot(normal, r.direction) > 0) // Back culling
        return false;

    if (!intersect_plane(r, a, normal, &hit))
        return false;

    float3 tmp = cross(b - a, hit.position - a);
    if (dot(hit.normal, tmp) < 0)
        return false;

    tmp = cross(c - b, hit.position - b);
    if (dot(hit.normal, tmp) < 0)
        return false;

    tmp = cross(a - c, hit.position - c);
    if (dot(hit.normal, tmp) < 0)
        return false;

    *out = hit;
    return true;
}
