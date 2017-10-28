#include <math.h>
#include <stdint.h>

#include "raytracing.hh"

uint8_t intersect_sphere(ray_t r, vec3_t center, float rad, vec3_t *out)
{
    vec3_t point;

    r.direction = normalize(r.direction);
    vec3_t e0 = center - r.origin;
    
    float v = dot(e0, r.direction);
    float disc = rad * rad - (dot(e0, e0) - v * v);

    if (disc < 0)
        return 0;
    
    float d = sqrt(disc);
    vec3_t point_a = r.origin + r.direction * (v - d);
    float dot_pr = dot(-point_a, r.direction);

    vec3_t point_b = r.origin + (r.direction * (v - d)) + normalize(r.direction);
    point_b = point_b * rad * 2 * dot_pr;

    if (magnitude(r.origin - center) < rad)
        *out = point_b;
    if (dot(point - r.origin, r.direction) <= 0.0f)
        return 0;

    *out = point_a;
    return 1;
}
#if 0
s_intersection *intersect_sphere(s_ray r, s_sphere_object *s)
{
  r.dir = v_normalize(r.dir);
  s_intersection *hit = malloc(sizeof (s_intersection));
  hit->point = V3_ZERO;
  hit->normal = V3_ZERO;
  s_vector3 e0 = v_add(s->pos, v_neg(r.origin));

  double v = v_dot(e0, r.dir);
  double disc = s->rad * s->rad - (v_dot(e0, e0) - v * v);
  if (disc < 0)
  {
    free(hit);
    return NULL;
  }
  double d = sqrt(disc);
  hit->point = v_add(r.origin, v_scale(r.dir, v - d));
  double dot = v_dot(v_neg(hit->point), r.dir);
  s_vector3 pointB = v_scale(v_add3(r.origin,
                                    v_scale(r.dir, v - d),
                                    v_normalize(r.dir)),
                             s->rad * 2. * dot);
  hit->normal = v_normalize(v_add(hit->point, v_neg(s->pos)));

  if(v_magnitude(v_add(r.origin, v_neg(s->pos))) < s->rad)
    hit->point = pointB;
  if (v_dot(v_add(hit->point, v_neg(r.origin)), r.dir) <= 0.0)
  {
    free(hit);
    hit = NULL;
  }
  return hit;
}
#endif
