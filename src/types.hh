#pragma once

#include <stdint.h>
#include <vector>

#include "vectors.hh"

namespace pathtracer
{
    typedef enum object_type {
        SPHERE, PLANE, MESH, AREA_LIGHT
    } object_type_e;

    typedef struct object {
        object_type_e type;

        vec3_t position;
        vec3_t rotation;
        vec3_t color;
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
}
