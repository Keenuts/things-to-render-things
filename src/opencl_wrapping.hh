#pragma once

#include <assert.h>
#include <CL/cl.hpp>
#include <stdio.h>
#include <fstream>

#include "defines.hh"
#include "framework.hh"
#include "renderer.hh"

namespace RE
{
#if 0
    struct kernel_info {
        cl_int samples;
        cl_int depth;
        cl_int block_width;
        cl_int block_height;
        cl_int width;
        cl_int height;

        cl_float3 camera_position;
        cl_float3 camera_direction;
        cl_float3 camera_up;
        cl_float3 camera_right;

        ulong object_count;
        ulong vtx_count;
    };

    struct kernel_object {
        object_type_e type;
        cl_float3 position;
        cl_float3 rotation;
        cl_float3 color;

        float radius;
        cl_float3 size;
        cl_float3 normal;
        ulong vtx_index;
        ulong vtx_count;
    };

    struct kernel_parameters {
        struct kernel_info info;
        cl::Buffer objects;
        cl::Buffer vertex;
        cl::Image2D image;
    };

    void pathtracer_gpu(context_t& ctx, uint32_t width, uint32_t height);
#endif
}
