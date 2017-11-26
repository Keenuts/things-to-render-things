#pragma once

#include <stdint.h>

#include "types.hh"
#include "framework.hh"
#include "raytracing.hh"

namespace RE
{
    ray_t get_ray_from_camera(struct renderer_info& i, uint32_t x, uint32_t y);

    vec3_t pathtrace(scene_t *scene, ray_t ray);
    vec3_t raytrace(scene_t *scene, ray_t ray, uint32_t bounce);

    void mdt_generate_irradiance_lights(scene_t *scene);
    vec3_t mdt(scene_t *scene, ray_t ray);

    vec3_t bidir_pathtrace(scene_t *scene, ray_t ray);
}
