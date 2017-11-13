#pragma once

/* Object related definitions */

typedef enum object_type {
    SPHERE, PLANE, MESH, AREA_LIGHT
} object_type_e;

typedef struct object {
    object_type_e type;

    double3 position;
    double3 rotation;
    double3 color;

    double radius;
    double3 size;
    double3 normal;
    ulong vtx_index;
    ulong vtx_count;
} object_t;

/* End of object related definitions */

typedef struct ray {
    double3 origin;
    double3 direction;
} ray_t;

typedef struct hit {
    double3 position;
    double3 normal;
    object_t object;
} hit_t;

struct kernel_info {
    int block_width;
    int block_height;
    int width;
    int height;

    double3 camera_position;
    double3 camera_direction;
    double3 camera_up;
    double3 camera_right;

    ulong object_count;
    ulong vtx_count;
};

typedef struct scene {
    double3 camera_position;
    double3 camera_direction;
    double3 camera_up;
    double3 camera_right;

    ulong object_count;
    __global object_t *objects;

    // Used for the PRNG
    int last_seed;
} scene_t;
