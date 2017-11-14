#include "types.hcl"
#include "helpers.hcl"
#include "raytracing.hcl"

#define SAMPLES_COUNT 1024
#define SAMPLE_DEPTH 4

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE
                             | CLK_ADDRESS_CLAMP_TO_EDGE
                             | CLK_FILTER_NEAREST;

static inline
bool intersect_with_plane(object_t o, ray_t r, hit_t *out)
{
    return intersect_plane(r, o.position, rotation(o.normal, o.rotation), out);
}

static inline
bool intersect_with_area_light(object_t o, ray_t r, hit_t *out)
{
    hit_t hit;

    float3 vt = rotation(o.size, o.rotation);
    float3 a = (float3)(-vt.x * 0.5f, 0.0f, -vt.z * 0.5f);
    float3 b = (float3)(-vt.x * 0.5f, 0.0f,  vt.z * 0.5f);
    float3 c = (float3)( vt.x * 0.5f, 0.0f,  vt.z * 0.5f);
    float3 d = (float3)( vt.x * 0.5f, 0.0f, -vt.z * 0.5f);

    a = a + o.position;
    b = b + o.position;
    c = c + o.position;
    d = d + o.position;

    if (intersect_tri(r, a, d, c, out))
        return true;

    if (intersect_tri(r, a, c, b, out))
        return true;

    return false;
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

static float3 render_ray(scene_t *scene, ray_t ray)
{
    float3 mask = (float3)(1.0f, 1.0f, 1.0f);
    float3 color = (float3)(0.0f, 0.0f, 0.0f);

    for (uint i = 0; i < SAMPLE_DEPTH; i++) {
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
  struct kernel_info info,
  __global object_t *objects,
  __global float3 *vertex,
  __write_only image2d_t output
)
{
  const int2 pos = (int2)(
        get_group_id(0) * get_local_size(0) + get_local_id(0) * info.block_width,
        get_group_id(1) * get_local_size(1) + get_local_id(1) * info.block_height
  );

  scene_t scene;
  scene.camera_position  = info.camera_position;
  scene.camera_direction = info.camera_direction;
  scene.camera_up        = info.camera_up;
  scene.camera_right     = info.camera_up;
  scene.object_count     = info.object_count;

  scene.objects = objects;

  for (int y = 0; y < info.block_height; y++) {
    for (int x = 0; x < info.block_width; x++) {
      int2 local_pos = (int2)(pos.x + x, pos.y + y);

      int2 resolution = (int2)(info.width, info.height);
      ray_t r = compute_ray(scene, resolution, local_pos);

      float3 color = (float3)(0, 0, 0);

      for (uint i = 0; i < SAMPLES_COUNT; i++)
        color += render_ray(&scene, r) * (1.0f / SAMPLES_COUNT);

      color = saturate(color);

      uint4 i_color = (uint4)(
        (uint)(color.x * 255),
        (uint)(color.y * 255),
        (uint)(color.z * 255),
        255
      );

      write_imageui(output, local_pos, i_color);
    }
  }
}
