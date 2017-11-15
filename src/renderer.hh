#pragma once

#include <stdint.h>

#include "types.hh"
#include "framework.hh"
#include "raytracing.hh"

namespace RE
{
    ray_t get_ray_from_camera(context_t& ctx, uint32_t x, uint32_t y);

    vec3_t pathtrace(scene_t *scene, ray_t ray, uint32_t bounce);
    vec3_t raytrace(scene_t *scene, ray_t ray, uint32_t bounce);

    light_t* mdt_generate_irradiance_lights(scene_t *scene, uint64_t *count);
    vec3_t mdt(scene_t *scene, ray_t ray, light_t *lights, uint64_t l_count);
}
