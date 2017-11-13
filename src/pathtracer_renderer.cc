#include <cassert>
#include <limits>
#include <string.h>

#include "defines.hh"
#include "helpers.hh"
#include "pathtracer_renderer.hh"
#include "raytracing.hh"

namespace pathtracer
{
    ray_t get_ray_from_camera(context_t& ctx, uint32_t x, uint32_t y)
    {
        scene_t *scene = ctx.scene;
        double width = ctx.width;
        double height = ctx.height;
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

    static bool intersect_sphere(object_sphere_t *o, ray_t r, hit_t *out)
    {
        return intersect_sphere(r, o->position, o->radius, out);
    }

    static bool intersect_plane(object_plane_t *o, ray_t r, hit_t *out)
    {
        return intersect_plane(r, o->position, rotate(o->normal, o->rotation), out);
    }

    static bool intersect_mesh(object_mesh_t *o, ray_t r, hit_t *out)
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

    static bool intersect_area_light(area_light_t *o, ray_t r, hit_t *out)
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

    static bool intersect_scene(scene_t *scene, ray_t ray, hit_t *out)
    {
        hit_t hit;
        double depth = std::numeric_limits<double>::infinity();
        bool touch = false;

        for (object_t *o : scene->objects) {
            hit_t local_hit;
            bool local_touch = false;

            switch (o->type) {
                case object_type_e::SPHERE:
                    local_touch = intersect_sphere((object_sphere_t*)o, ray, &local_hit);
                    break;
                case object_type_e::PLANE:
                    local_touch = intersect_plane((object_plane_t*)o, ray, &local_hit);
                    break;
                case object_type_e::MESH:
                    local_touch = intersect_mesh((object_mesh_t*)o, ray, &local_hit);
                    break;
                case object_type_e::AREA_LIGHT:
                    local_touch = intersect_area_light((area_light_t*)o, ray, &local_hit);
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

    vec3_t render_ray(scene_t *scene, ray_t ray, uint32_t bounce)
    {
        vec3_t luminance(0.0, 0.0, 0.0);
        uint32_t samples = 1;

        for (uint32_t i = 0; i < SAMPLES_COUNT; i++) {
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
                vec3_t ir = render_ray(scene, n_ray, bounce + 1)
                            * dot(hit.normal, n_ray.direction);
                sample_luminance = PI * 2.0 * BRDF * ir;
            }
            else
                sample_luminance = BRDF;
            luminance = luminance + sample_luminance;
        }

        return saturate(luminance / samples);
    }
}
