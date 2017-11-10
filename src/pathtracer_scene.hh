#pragma once

#include <stdint.h>
#include <vector>

#include "vectors.hh"

namespace pathtracer
{
    typedef enum object_type {
        SPHERE,
    } object_type_e;

    typedef struct object {
        object_type_e type;

        vec3_t position;
        vec3_t color;
    } object_t;

    typedef struct object_sphere : public object_t {
        float radius;
    } object_sphere_t;

    typedef struct scene {
        uint32_t width;
        uint32_t height;

        vec3_t camera_position;
        vec3_t camera_direction;

        std::vector<object_t*> objects;
    } scene_t;
}
