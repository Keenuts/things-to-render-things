#pragma once

#include <stdint.h>

#include "pathtracer_scene.hh"
#include "raytracing.hh"

namespace pathtracer
{
    typedef struct render_info {
        uint8_t* output_frame;
        float* depth_buffer;

        uint32_t width;
        uint32_t height;
    } render_info_t;

    void render_scene(scene_t *scene);
    void render_sphere(render_info_t *output, object_t *object, ray_t ray);
}
