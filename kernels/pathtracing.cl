#include "raytracing.hcl"
#include "types.hcl"

#define SAMPLES_COUNT 30
#define SAMPLE_DEPTH 4

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE
                             | CLK_ADDRESS_CLAMP_TO_EDGE
                             | CLK_FILTER_NEAREST;

static bool intersect_with_plane(object_t o, ray_t r, hit_t *out)
{
    //return intersect_plane(r, o.position, rotation(o.normal, o.rotation), out);
    return intersect_plane(r, o.position, o.normal, out);
}

static bool intersect_with_area_light(object_t o, ray_t r, hit_t *out)
{
    hit_t hit;

    double3 vt = o.size; //rotation(o.size, o.rotation);
    double3 a = (double3)(-vt.x * 0.5, 0, -vt.z * 0.5);
    double3 b = (double3)(-vt.x * 0.5, 0,  vt.z * 0.5);
    double3 c = (double3)( vt.x * 0.5, 0,  vt.z * 0.5);
    double3 d = (double3)( vt.x * 0.5, 0, -vt.z * 0.5);

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
    double depth = INFINITY;
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

        double tmp_depth = length(local_hit.position - ray.origin);

        if (tmp_depth < depth) {
            hit = local_hit;
            hit.object = o;
            depth = tmp_depth;
        }
    }

    *out = hit;
    return touch;
}

#define POP(Stack, Rsp) stack[--rsp]
#define PUSH(Stack, Rsp, Data) stack[rsp++] = Data
#define BLACK (double3)(0.0, 0.0, 0.0)
#define STACK_MAX (SAMPLE_DEPTH + 1)

static double3 render_ray(scene_t *scene, ray_t ray)
{
    double3 stack[STACK_MAX];
    ulong rsp = 0;

    while (true) {
        hit_t hit;

        if (!intersect_scene(scene, ray, &hit)) {
            PUSH(stack, rsp, BLACK);
            break;
        }

        if (hit.object.type == AREA_LIGHT) {
            PUSH(stack, rsp, hit.object.color);
            break;
        }

        ray_t n_ray;
        n_ray.direction = get_hemisphere_random(scene, hit.normal);
        n_ray.origin = hit.position + hit.normal * D_EPSYLON;

        double3 BRDF = hit.object.color / PI;
        BRDF = dot(hit.normal, n_ray.direction) * 2.0 * PI * BRDF;
        PUSH(stack, rsp, saturate(BRDF));

        if (rsp >= SAMPLE_DEPTH)
            break;

        ray = n_ray;
    }

    double3 result = (double3)(1.0, 1.0, 1.0);
    do {
        result *= POP(stack, rsp);
    } while (rsp > 0);

    return saturate(result);
}

#undef POP
#undef PUSH
#undef BLACK
#undef STACK_MAX

__kernel void pathtracer(
  struct kernel_info info,
  __global object_t *objects,
  __global double3 *vertex,
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

  for (int y = 0; y < info.block_width; y++) {
    for (int x = 0; x < info.block_height; x++) {
      int2 local_pos = (int2)(pos.x + x, pos.y + y);

      int2 resolution = (int2)(info.width, info.height);
      ray_t r = compute_ray(scene, resolution, local_pos);

      double3 sum = (double3)(0, 0, 0);

      for (uint i = 0; i < SAMPLES_COUNT; i++) {
        double3 c = render_ray(&scene, r);
        sum += c;
      }

      double3 color = saturate(sum / SAMPLES_COUNT);

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
