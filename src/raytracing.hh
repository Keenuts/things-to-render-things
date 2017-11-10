#pragma once

#include "vectors.hh"

typedef struct ray
{
    vec3_t origin;
    vec3_t direction;
} ray_t;

uint8_t intersect_sphere(ray_t r, vec3_t center, double rad, vec3_t *out);
