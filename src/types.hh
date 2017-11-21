#pragma once

#include <stdint.h>
#include <vector>

#include "vectors.hh"

namespace RE
{
    typedef struct material {
        vec3_t emission;
        vec3_t diffuse;
    } material_t;

    typedef enum object_type {
        SPHERE, PLANE, MESH, AREA_LIGHT
    } object_type_e;

    typedef struct object {
        object_type_e type;

        vec3_t position;
        vec3_t rotation;
        struct material mlt;
    } object_t;

    typedef struct light : public object_t {
    } light_t;

    // Objects

    typedef struct object_sphere : public object_t {
        float radius;
    } object_sphere_t;

    typedef struct object_mesh : public object_t {
        vec3_t *vtx;
        uint64_t vtx_count;
    } object_mesh_t;

    typedef struct object_plane : public object_t {
        vec3_t normal;
    } object_plane_t;

    // Lights

    typedef struct dir_light : public light_t {
        vec3_t direction;
    } dir_light_t;

    typedef struct area_light : public light_t {
        vec3_t size;
        vec3_t normal;
    } area_light_t;

    // Scene

    typedef struct scene {
        vec3_t camera_position;
        vec3_t camera_direction;

        std::vector<object_t*> objects;
        std::vector<light_t*> lights;
    } scene_t;

    struct kernel_info {
        int samples;
        int depth;

        int offset_x;
        int offset_y;
        int stride;
        int width;
        int height;

        cl_float3 camera_position;
        cl_float3 camera_direction;
        cl_float3 camera_up;
        cl_float3 camera_right;

        ulong object_count;
        ulong vtx_count;
    };

    struct kernel_material {
        cl_float3 diffuse;
        cl_float3 emission;
    };

    struct kernel_object {
        object_type_e type;
        cl_float3 position;
        cl_float3 rotation;
        struct kernel_material mlt;

        float radius;
        cl_float3 size;
        cl_float3 normal;
        ulong vtx_index;
        ulong vtx_count;
    };

    struct kernel_parameters {
        struct kernel_info info;
        cl_mem objects;
        cl_mem vertex;
    };
}
