#pragma once

#include <CL/cl.hpp>
#include <stdint.h>

#include "pathtracer_scene.hh"
#include "raytracing.hh"

namespace pathtracer
{
    typedef struct context {
        uint32_t width;
        uint32_t height;
        uint8_t *output_frame;

        cl::Context opencl_ctx;
        cl::Device device;
        cl::Platform platform;
        cl::Program kernel;
        scene_t *scene;
    } context_t;


    void render_scene(scene_t *scene, uint32_t width, uint32_t height);
    void render_scene_chunk(context_t ctx, vec3_t size, vec3_t block_pos);
}
