#include <cassert>
#include <limits>
#include <string.h>

#include "defines.hh"
#include "helpers.hh"
#include "mapping.hh"
#include "raytracing.hh"
#include "renderer.hh"

namespace RE
{
    ray_t get_ray_from_camera(struct renderer_info& i, uint32_t x, uint32_t y)
    {
        scene_t *scene = i.scene;
        float width = i.width;
        float height = i.height;
        const float FOV = 45.0;

        float L = width / (tan(DEG2RAD * FOV * 0.5) * 2.0);

        vec3_t middle = scene->camera_position + scene->camera_direction * L;
        float s_width = lerp(-width, width, x / width) / 2.0;
        float s_height = lerp(-height, height, y / height) / 2.0;

        vec3_t screen_ptr = VECTOR_RIGHT * s_width
                      - VECTOR_UP * s_height
                      + middle;

        vec3_t direction = screen_ptr - scene->camera_position;
        vec3_t origin = scene->camera_position;

        return { origin, normalize(direction) };
    }

    static bool intersect_sphere(object_sphere_t *o, ray_t r, hit_t *out)
    {
        bool touch = intersect_sphere(r, o->position, o->radius, out);
        if (touch)
            out->uv_coord = get_sphere_uv(o->position, out->position);
        return touch;

    }

    static bool intersect_plane(object_plane_t *o, ray_t r, hit_t *out)
    {
        return intersect_plane(r, o->position, rotate(o->normal, o->rotation), out);
    }

    static bool intersect_mesh(object_mesh_t *o, ray_t r, hit_t *out)
    {
        hit_t hit;
        bool touch = false;
        float depth = std::numeric_limits<float>::infinity();

        assert(o->vtx_count > 0 && "An empty mesh is in the rendering system");
        assert(o->vtx_count % 3 == 0 && "Invalid vtx count. Must be multiple of 3");

        for (uint64_t i = 0; i < o->vtx_count; i += 3) {
            hit_t local_hit;

            vec3_t vtx[3]  = {
                rotate(o->vtx[i + 0], o->rotation) + o->position,
                rotate(o->vtx[i + 1], o->rotation) + o->position,
                rotate(o->vtx[i + 2], o->rotation) + o->position
            };

            if (!intersect_tri(r, vtx[0], vtx[1], vtx[2], &local_hit))
                continue;

            touch = true;
            float local_depth = magnitude(local_hit.position - r.origin);
            if (local_depth < depth) {
                hit = local_hit;
                hit.uv_coord = get_triangle_uv(vtx, o->uv + i, hit.position);
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
        float depth = std::numeric_limits<float>::infinity();
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

            float tmp_depth = magnitude(local_hit.position - ray.origin);

            if (tmp_depth < depth) {
                hit = local_hit;
                hit.object = o;
                depth = tmp_depth;
            }
        }

        *out = hit;
        return touch;
    }

    vec3_t raytrace(scene_t *scene, ray_t ray, uint32_t bounce)
    {
        vec3_t luminance(0.1, 0.1, 0.1);
        hit_t hit;

        if (!intersect_scene(scene, ray, &hit))
            return luminance;

        if (hit.object->type == object_type_e::AREA_LIGHT)
            return hit.object->mlt.emission;

#if defined(RT_ENABLE_SHADOWS)
        vec3_t light = vec3_t(0.0);

        for (object_t *o : scene->objects) {
            if (o->type != object_type_e::AREA_LIGHT)
                continue;

             vec3_t l = vec3_t(0.0);
             for (uint32_t i = 0; i < RT_SOFT_SHADOW_SAMPLES; i++) {
                 ray_t r;
                 r.origin = hit.position + hit.normal * F_EPSYLON;
                 vec3_t d1 = normalize(o->position - hit.position);
                 vec3_t d2 = get_hemisphere_random(hit.normal);

                 r.direction = lerp(d1, d2, RT_SOFT_SHADOW_RADIUS);
                 hit_t h;

                 if (!intersect_scene(scene, r, &h))
                     continue;
                 if (h.object->type != object_type_e::AREA_LIGHT)
                     continue;

                 l = l + h.object->mlt.emission * (1.0f / RT_SOFT_SHADOW_SAMPLES);
             }
             light = light + l;
        }
        return get_diffuse_color(scene, hit) * light;
#else
        return get_diffuse_color(scene, hit);
#endif
    }

    vec3_t pathtrace(scene_t *scene, ray_t ray)
    {
        vec3_t mask = WHITE;
        vec3_t color = BLACK;

        for (uint32_t i = 0; i < PT_MAX_DEPTH; i++) {
            hit_t hit;

            if (!intersect_scene(scene, ray, &hit)) {
                color = BLACK;
                break;
            }

            if (hit.object->type == object_type_e::AREA_LIGHT) {
                area_light_t *l = static_cast<area_light_t*>(hit.object);
                color += mask * l->mlt.emission * l->power;
                break;
            }

            vec3_t nl = hit.normal;
            nl *= dot(hit.normal, ray.direction) < 0 ? 1.0f : -1.0f;

            ray.direction = get_hemisphere_random(hit.normal);
            ray.origin = hit.position + hit.normal * F_EPSYLON;

            mask *= get_diffuse_color(scene, hit);
            mask *= dot(ray.direction, nl);
            mask *= 2.0f;
        }

        return color;
    }

    static void mdt_light_cast(scene_t *scene, light_t *l, uint64_t depth)
    {
        for (uint64_t i = 0; i < IR_RAY_PER_LIGHT; i++) {
            ray_t r;
            hit_t hit;

            if (l->type == object_type_e::AREA_LIGHT) {
                area_light_t *al = static_cast<area_light_t*>(l);

                r.origin = al->position + al->normal * F_EPSYLON;
                r.direction = get_hemisphere_random(al->normal);
            }
            else {
                r.origin - l->position;
                r.direction = get_sphere_random();
            }

            if (!intersect_scene(scene, r, &hit)) {
                continue;
            }

            if (hit.object->type == object_type_e::AREA_LIGHT) {
                continue;
            }

            light_t new_light;
            float dist_to_light = magnitude(hit.position - r.origin);

            new_light.position = hit.position + hit.normal * 2.f * F_EPSYLON;
            new_light.mlt.emission = l->mlt.emission * get_diffuse_color(scene, hit);
            new_light.power = l->power / (dist_to_light * dist_to_light);

            scene->mdt_lights.push_back(new_light);

            if (depth < IR_RAY_DEPTH)
                mdt_light_cast(scene, &scene->mdt_lights[scene->mdt_lights.size() - 1], depth + 1);
        }
    }

    void mdt_generate_irradiance_lights(scene_t *scene)
    {
        for (object_t *o : scene->objects) {
            if (o->type != object_type_e::AREA_LIGHT)
                continue;
            mdt_light_cast(scene, static_cast<light_t*>(o), 1);
        }
        printf("Created %zu lights\n", scene->mdt_lights.size());
    }

    vec3_t mdt(scene_t *scene, ray_t ray)
    {
        hit_t hit;

        if (!intersect_scene(scene, ray, &hit))
            return vec3_t(0.0f, 0.0f, 0.0f);

        if (hit.object->type == object_type_e::AREA_LIGHT)
            return hit.object->mlt.emission;

        vec3_t light = BLACK;
        uint64_t l_count = scene->mdt_lights.size();

        for (uint64_t i = 0; i < l_count; i++) {
            hit_t l_hit;
            ray_t l_ray;

            l_ray.origin = hit.position + hit.normal * F_EPSYLON;
            l_ray.direction = normalize(scene->mdt_lights[i].position - l_ray.origin);

            if (!intersect_scene(scene, l_ray, &l_hit))
                continue;

            vec3_t oh = l_hit.position - l_ray.origin;
            vec3_t ol = scene->mdt_lights[i].position - l_ray.origin;
            float dist_to_light = magnitude(ol);
            if (magnitude(oh) < dist_to_light) // No direct sight
                continue;

            float factor = 1.f / l_count;
            light += scene->mdt_lights[i].mlt.emission * factor;
        }

        return saturate(light) * get_diffuse_color(scene, hit);
    }

    vec3_t bidir_pathtrace(scene_t *scene, ray_t ray)
    {
        vec3_t mask = WHITE;
        vec3_t color = BLACK;

        for (uint32_t i = 0; i < BDPT_MAX_CRAY_DEPTH; i++) {
            hit_t hit;

            if (!intersect_scene(scene, ray, &hit)) {
                color = BLACK;
                break;
            }

            if (hit.object->type == object_type_e::AREA_LIGHT) {
                area_light_t *l = static_cast<area_light_t*>(hit.object);
                color += mask * l->mlt.emission * l->power;
                break;
            }

            vec3_t nl = hit.normal;
            nl *= dot(hit.normal, ray.direction) < 0 ? 1.0f : -1.0f;

            ray.direction = get_hemisphere_random(hit.normal);
            ray.origin = hit.position + hit.normal * F_EPSYLON;

            mask *= get_diffuse_color(scene, hit);
            mask *= dot(ray.direction, nl);
            mask *= 2.0f;

            // If out of bounce, let's try to close the path
            if (i + 1 < BDPT_MAX_CRAY_DEPTH)
                continue;

            vec3_t light = BLACK;
            for (light_t l : scene->mdt_lights) {
                hit_t l_hit;
                ray_t l_ray;

                l_ray.origin = hit.position + hit.normal * F_EPSYLON;
                l_ray.direction = normalize(l.position - l_ray.origin);

                if (!intersect_scene(scene, l_ray, &l_hit))
                    continue;

                vec3_t oh = l_hit.position - l_ray.origin;
                vec3_t ol = l.position - l_ray.origin;
                float dist_to_light = magnitude(ol);
                if (magnitude(oh) < dist_to_light) // No direct sight
                    continue;

                float factor = 1.f / scene->mdt_lights.size();
                light += l.mlt.emission * factor;
            }

            color += mask * light;
        }

        return color;
    }
}
