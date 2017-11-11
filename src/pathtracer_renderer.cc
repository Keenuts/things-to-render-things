#include <stdio.h>
#include <string.h>
#include <limits>
#include <cassert>

#include "lodepng.hh"
#include "pathtracer_renderer.hh"
#include "raytracing.hh"
#include "helpers.hh"

#define STRIDE 4 //(BGR)

namespace pathtracer
{
    static ray_t create_ray_from_px(uint32_t x, uint32_t y, scene_t *scene)
    {
        double width = scene->width;
        double height = scene->height;
        const double FOV = 45.0;

        double L = width / (tan(DEG2RAD(FOV / 2.0)) * 2.0);

        vec3_t middle = scene->camera_position + scene->camera_direction * L;
        double s_width = lerp(-width, width, x / width) / 2.0;
        double s_height = lerp(-height, height, y / height) / 2.0;

        vec3_t screen_ptr = VECTOR_RIGHT * s_width
                      - VECTOR_UP * s_height
                      + middle;

        vec3_t direction = screen_ptr - scene->camera_position;
        vec3_t origin = scene->camera_position;

        return { origin, normalize(direction) };
    }

    void render_scene(scene_t *scene)
    {
        render_info_t info;
        memset(&info, 0, sizeof(render_info_t));

        info.width = scene->width;
        info.height = scene->height;

        info.output_frame = new uint8_t[info.width * info.height * STRIDE];
        info.depth_buffer = new float[info.width * info.height];

        
        for (uint32_t y = 0; y < info.height; y++) {
            for (uint32_t x = 0; x < info.width; x++) {

                ray_t r = create_ray_from_px(x, y, scene);

                vec3_t color = render_pixel(scene, r);

                info.output_frame[(info.width * y + x) * STRIDE + 0] = color.r * 255.0;
                info.output_frame[(info.width * y + x) * STRIDE + 1] = color.g * 255.0;
                info.output_frame[(info.width * y + x) * STRIDE + 2] = color.b * 255.0;
                info.output_frame[(info.width * y + x) * STRIDE + 3] = 255;
            }

            float percent = y / (float)(info.height);
            printf("\rprogress: %.2f%%    ", percent * 100.0f);
        }
        printf("\rprogress: 100%%    \n");

        printf("[ OK ] Rendering done.\n");

        lodepng::encode("output.png", info.output_frame, info.width, info.height);
        printf("[ OK ] Output written to disk.\n");

        delete[] info.output_frame;
        delete[] info.depth_buffer;
        printf("[ OK ] Memory freed.\n");
    }

    vec3_t render_pixel(scene_t *scene, ray_t ray, uint32_t bounce)
    {
        double depth = std::numeric_limits<double>::infinity();
        vec3_t color(0, 0, 0);
        vec3_t light = normalize(vec3_t(0.3, -1.0, 0.2));

        for (object_t *o : scene->objects) {
            hit_t hit;
            bool touch;

            switch (o->type) {
                case object_type_e::SPHERE:
                    touch = intersect_object(scene, (object_sphere_t*)o, ray, &hit);
                    break;
                case object_type_e::PLANE:
                    touch = intersect_object(scene, (object_plane_t*)o, ray, &hit);
                    break;
                case object_type_e::MESH:
                    touch = intersect_object(scene, (object_mesh_t*)o, ray, &hit);
                    break;
                default:
                    assert(0 && "Object type unknown.");
            };

            if (!touch)
                continue;

            double tmp_depth = magnitude(hit.position - ray.origin);

            if (tmp_depth < depth) {
                color = std::max(0.2, dot(hit.normal, -light)) * o->color;

                if (bounce < 2 && 0) {
                    ray_t refl_ray;

                    refl_ray.origin = hit.position + hit.normal * 0.1;
                    refl_ray.direction = reflect(ray.direction, hit.normal);

                    vec3_t color_b = render_pixel(scene, refl_ray, bounce + 1);
                    color = saturate(color_b * 0.4 + color * 0.6);
                }
                depth = tmp_depth;
            }
        }

        return color;
    }

    bool intersect_object(scene_t *scene, object_sphere_t *o, ray_t r, hit_t *out)
    {
        return intersect_sphere(r, o->position, o->radius, out);
    }

    bool intersect_object(scene_t *scene, object_plane_t *o, ray_t r, hit_t *out)
    {
        return intersect_plane(r, o->position, rotate(o->normal, o->rotation), out);
    }

    bool intersect_object(scene_t *scene, object_mesh_t *o, ray_t r, hit_t *out)
    {
        hit_t hit;
        bool touch = false;
        double depth = std::numeric_limits<double>::infinity();

        assert(o->vtx_count % 3 == 0 && "Invalid vtx count. Must be multiple of 3");

        for (uint64_t i = 0; i < o->vtx_count; i += 3) {
            hit_t local_hit;

            vec3_t a = rotate(o->vtx[i + 0], o->rotation) + o->position;
            vec3_t b = rotate(o->vtx[i + 1], o->rotation) + o->position;
            vec3_t c = rotate(o->vtx[i + 2], o->rotation) + o->position;

            if (!intersect_tri(r, a, b, c, &local_hit))
                continue;

            touch = true;
            double local_depth = magnitude(local_hit.position - r.origin);
            if (local_depth < depth) {
                hit = local_hit;
                depth = local_depth;
            }
        }

        *out = hit;
        return touch;
    }
}
