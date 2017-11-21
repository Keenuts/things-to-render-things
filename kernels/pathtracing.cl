#include "types.hcl"
#include "helpers.hcl"
#include "raytracing.hcl"

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE
| CLK_ADDRESS_CLAMP_TO_EDGE
| CLK_FILTER_NEAREST;

static inline
bool intersect_with_plane(object_t o, ray_t r, hit_t *out)
{
    return intersect_plane(r, o.position, rotation(o.normal, o.rotation), out);
}

static bool intersect_scene(scene_t *scene, ray_t ray, hit_t *out)
{
    hit_t hit;
    float depth = INFINITY;
    bool touch = false;

    for (ulong i = 0; i < scene->object_count; i++) {
        object_t o = scene->objects[i];
        hit_t local_hit;
        bool local_touch = false;

        switch (o.type) {
            case SPHERE:
                local_touch = intersect_sphere(ray, o.position, o.radius, &local_hit);
                break;
            case PLANE:
                local_touch = intersect_with_plane(o, ray, &local_hit);
                break;
            case MESH:
                return false;
                break;
            case AREA_LIGHT:
                local_touch = intersect_with_area_light(o, ray, &local_hit);
                break;
            default:
                return false;
        };

        if (!local_touch)
            continue;
        touch = true;

        float tmp_depth = length(local_hit.position - ray.origin);

        if (tmp_depth < depth) {
            hit = local_hit;
            hit.object = o;
            depth = tmp_depth;
        }
    }

    *out = hit;
    return touch;
}

#define BLACK (float3)(0.0f, 0.0f, 0.0f)

static float3 render_ray(scene_t *scene, ray_t ray, int max_depth)
{
    float3 mask = (float3)(1.0f, 1.0f, 1.0f);
    float3 color = (float3)(0.0f, 0.0f, 0.0f);

    for (uint i = 0; i < max_depth; i++) {
        hit_t hit;

        if (!intersect_scene(scene, ray, &hit)) {
            color = BLACK;
            break;
        }

        if (hit.object.type == AREA_LIGHT) {
            color += mask * hit.object.color * 3.0f;
            break;
        }

        float3 nl = dot(hit.normal, ray.direction) < 0 ? hit.normal : hit.normal * -1;

        ray.direction = get_hemisphere_random(scene, hit.normal);
        ray.origin = hit.position + hit.normal * F_EPSYLON;

        mask *= hit.object.color;
        mask *= dot(ray.direction, nl);
        mask *= 2.0f;
    }

    return color;
}

#undef BLACK

__kernel void pathtracer(
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

    float3 color = (float3)(0, 0, 0);

    for (uint i = 0; i < info.samples; i++) {
        color += render_ray(&scene, r, info.depth) * (1.0f / info.samples);

        color = saturate(color);

        uint4 i_color = (uint4)(
                (uint)(color.x * 255),
                (uint)(color.y * 255),
                (uint)(color.z * 255),
                255
                );

        output[id + 0] = i_color.x;
        output[id + 1] = i_color.y;
        output[id + 2] = i_color.z;
        output[id + 3] = 255;
    }
}
