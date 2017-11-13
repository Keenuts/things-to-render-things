#pragma once

#include <assert.h>
#include <CL/cl.hpp>
#include <stdio.h>
#include <fstream>

#include "defines.hh"
#include "pathtracer_framework.hh"
#include "pathtracer_renderer.hh"

namespace pathtracer
{
    struct kernel_info {
        int block_width;
        int block_height;
        int width;
        int height;

        cl_double3 camera_position;
        cl_double3 camera_direction;
        cl_double3 camera_up;
        cl_double3 camera_right;

        uint64_t object_count;
        uint64_t vtx_count;
    };

    struct kernel_object {
        object_type_e type;
        cl_double3 position;
        cl_double3 rotation;
        cl_double3 color;

        double radius;
        cl_double3 size;
        cl_double3 normal;
        uint64_t vtx_index;
        uint64_t vtx_count;
    };

    struct kernel_parameters {
        struct kernel_info info;
        cl::Buffer objects;
        cl::Buffer vertex;
        cl::Image2D image;
    };

    void pathtracer_gpu(context_t& ctx, uint32_t width, uint32_t height);
}
