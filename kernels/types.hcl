#pragma once

/* Object related definitions */

typedef enum object_type {
    SPHERE, PLANE, MESH, AREA_LIGHT
} object_type_e;

typedef struct object {
    object_type_e type;

    float3 position;
    float3 rotation;
    float3 color;

    float radius;
    float3 size;
    float3 normal;
    ulong vtx_index;
    ulong vtx_count;
} object_t;

/* End of object related definitions */

typedef struct ray {
    float3 origin;
    float3 direction;
} ray_t;

typedef struct hit {
    float3 position;
    float3 normal;
    object_t object;
} hit_t;

struct kernel_info {
    int samples;
    int depth;
    int block_width;
    int block_height;
    int width;
    int height;

    float3 camera_position;
    float3 camera_direction;
    float3 camera_up;
    float3 camera_right;

    ulong object_count;
    ulong vtx_count;
};

typedef struct scene {
    float3 camera_position;
    float3 camera_direction;
    float3 camera_up;
    float3 camera_right;

    ulong object_count;
    __global object_t *objects;

    // Used for the PRNG
    int last_seed;
} scene_t;
