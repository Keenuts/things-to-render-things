#pragma once

#include "defines.hh"
#include "raytracing.hh"
#include "types.hh"
#include "vectors.hh"

namespace RE
{
    vec3_t get_diffuse_color(scene_t *scene, hit_t& hit);

    vec3_t get_triangle_uv(vec3_t vtx[3], vec3_t uv[3], vec3_t pt);
    vec3_t get_sphere_uv(vec3_t center, vec3_t pt);
}
