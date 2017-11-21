#include "defines.hcl"
#include "types.hcl"
#include "helpers.hcl"
#include "raytracing.hcl"
#include "raytracer.hcl"

__kernel void raytracer(
    global uchar *output, 
    struct kernel_info info,
    __global object_t *objects,
    __global float3 *vertex)
{
    int x = info.offset_x + get_global_id(0);
    int y = info.offset_y + get_global_id(1);

    int id = (x + y * info.width) * info.stride;


    scene_t scene;
    scene.camera_position  = info.camera_position;
    scene.camera_direction = info.camera_direction;
    scene.camera_up        = info.camera_up;
    scene.camera_right     = info.camera_up;
    scene.object_count     = info.object_count;

    scene.objects = objects;

    int2 resolution = (int2)(info.width, info.height);
    ray_t r = compute_ray(scene, resolution, (int2)(x, y));

    float3 color = raytracer_render_ray(&scene, r, info.depth);
    color = saturate(color);

    output[id + 0] = color.x * 255;
    output[id + 1] = color.y * 255;
    output[id + 2] = color.z * 255;
    output[id + 3] = 255;
}
