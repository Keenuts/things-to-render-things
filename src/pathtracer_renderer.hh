#pragma once

#include <stdint.h>

#include "pathtracer_scene.hh"
#include "pathtracer_framework.hh"
#include "raytracing.hh"

namespace pathtracer
{
    ray_t get_ray_from_camera(context_t& ctx, uint32_t x, uint32_t y);
    vec3_t render_ray(scene_t *scene, ray_t ray, uint32_t bounce);
}
