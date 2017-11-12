#include <stdio.h>
#include <string.h>
#include <limits>
#include <cassert>
#include <thread>

#include "defines.hh"
#include "helpers.hh"
#include "lodepng.hh"
#include "pathtracer_renderer.hh"
#include "raytracing.hh"

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

                vec3_t color = render_pixel(scene, r, 0);

                info.output_frame[(info.width * y + x) * STRIDE + 0] = color.r * 255.0;
                info.output_frame[(info.width * y + x) * STRIDE + 1] = color.g * 255.0;
                info.output_frame[(info.width * y + x) * STRIDE + 2] = color.b * 255.0;
                info.output_frame[(info.width * y + x) * STRIDE + 3] = 255;

            }
            float percent = (y * info.width) / (float)(info.height * info.width);
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

    vec3_t compute_dir_light(scene_t *scene, dir_light_t *light, hit_t hit)
    {
        return std::max(0.0, dot(hit.normal, -light->direction)) * light->color;
    }

    vec3_t compute_area_light(scene_t *scene, area_light_t *light, hit_t hit)
    {
        hit_t l_hit;
        ray_t r;
        r.origin = hit.position + hit.normal * 0.1;
        r.direction = normalize(light->position - hit.position);

        if (!intersect_scene(scene, r, &l_hit))
            return vec3_t(0, 0, 0);

        if ((void*)l_hit.object != (void*)light)
            return vec3_t(0, 0, 0);

        return std::max(0.0, dot(hit.normal, r.direction)) * light->color;
    }

    vec3_t compute_lighting(scene_t *scene, hit_t hit)
    {
        vec3_t light(0.1, 0.1, 0.1);

        for (light_t *l : scene->lights) {
            switch (l->type) {
                case object_type_e::DIR_LIGHT:
                    light = light + compute_dir_light(scene, (dir_light_t*)l, hit);
                    break;
                case object_type_e::AREA_LIGHT:
                    light = light + compute_area_light(scene, (area_light_t*)l, hit);
                    break;
                default:
                    assert(0 && "Object type unknown.");
            }
        }
        return saturate(saturate(light) * hit.object->color);
    }

    bool intersect_scene(scene_t *scene, ray_t ray, hit_t *out)
    {
        hit_t hit;
        double depth = std::numeric_limits<double>::infinity();
        bool touch = false;

        for (object_t *o : scene->objects) {
            hit_t local_hit;
            bool local_touch;

            switch (o->type) {
                case object_type_e::SPHERE:
                    local_touch = intersect_object(scene, (object_sphere_t*)o, ray, &local_hit);
                    break;
                case object_type_e::PLANE:
                    local_touch = intersect_object(scene, (object_plane_t*)o, ray, &local_hit);
                    break;
                case object_type_e::MESH:
                    local_touch = intersect_object(scene, (object_mesh_t*)o, ray, &local_hit);
                    break;
                case object_type_e::AREA_LIGHT:
                    local_touch = intersect_object(scene, (area_light_t*)o, ray, &local_hit);
                    break;
                default:
                    assert(0 && "Object type unknown.");
            };

            if (!local_touch)
                continue;
            touch = true;

            double tmp_depth = magnitude(local_hit.position - ray.origin);

            if (tmp_depth < depth) {
                hit = local_hit;
                hit.object = o;
                depth = tmp_depth;
            }
        }

        *out = hit;
        return touch;
    }

    vec3_t render_pixel(scene_t *scene, ray_t ray, uint32_t bounce)
    {
#if defined(RAYTRACING)
        hit_t hit;

        if (!intersect_scene(scene, ray, &hit))
            return vec3_t(0, 0, 0);

        vec3_t color;

        if (hit.object->type == object_type_e::AREA_LIGHT)
            color = hit.object->color;
        else
            color = compute_lighting(scene, hit);
        return color;
#else
        vec3_t luminance(0.0, 0.0, 0.0);
        uint32_t samples = 0;

        for (uint32_t i = 0; i < SAMPLES_COUNT / (bounce + 1); i++) {
            hit_t hit;
            vec3_t sample_luminance;

            if (!intersect_scene(scene, ray, &hit))
                continue;
            samples += 1;

            if (hit.object->type == object_type_e::AREA_LIGHT) {
                luminance = luminance + hit.object->color;
                break;
            }


            ray_t n_ray;
            n_ray.direction = get_hemisphere_random(hit.normal);
            n_ray.origin = hit.position + hit.normal * D_EPSYLON;
            
            vec3_t BRDF = hit.object->color / PI;
            vec3_t ir;

            if (bounce < SAMPLE_DEPTH) {
                vec3_t ir = render_pixel(scene, n_ray, bounce + 1)
                            * dot(hit.normal, n_ray.direction);
                sample_luminance = PI * 2.0 * BRDF * ir;
            }
            else
                sample_luminance = BRDF;
            luminance = luminance + sample_luminance;
        }

        if (samples == 0)
            return saturate(luminance);
        else
            return saturate(luminance / samples);
#endif
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

    bool intersect_object(scene_t *scene, area_light_t *o, ray_t r, hit_t *out)
    {
        hit_t hit;

        vec3_t vt = rotate(o->size, o->rotation);
        vec3_t a = vec3_t(-vt.x * 0.5, 0, -vt.z * 0.5);
        vec3_t b = vec3_t(-vt.x * 0.5, 0,  vt.z * 0.5);
        vec3_t c = vec3_t( vt.x * 0.5, 0,  vt.z * 0.5);
        vec3_t d = vec3_t( vt.x * 0.5, 0, -vt.z * 0.5);

        a = a + o->position;
        b = b + o->position;
        c = c + o->position;
        d = d + o->position;

        if (intersect_tri(r, a, d, c, out))
            return true;

        if (intersect_tri(r, a, c, b, out))
            return true;

        return false;
    }
}
