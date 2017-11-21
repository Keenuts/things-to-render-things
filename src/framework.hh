#pragma once

#include <CL/cl.h>
#include <stdint.h>

#include "types.hh"
#include "raytracing.hh"

namespace RE
{
    struct renderer_info {
        uint32_t width;
        uint32_t height;
        uint8_t *output_frame;
        scene_t *scene;
    };


    void render_scene(scene_t *scene, uint32_t width, uint32_t height);
}
