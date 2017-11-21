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

        cl_device_id device;
        cl_context context;
        cl_command_queue queue;
        cl_kernel kernel;
        cl_mem mem;

        scene_t *scene;
    };


    void render_scene(scene_t *scene, uint32_t width, uint32_t height);
    //void render_scene_chunk(context_t ctx, vec3_t size, vec3_t block_pos);
}
