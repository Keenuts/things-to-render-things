#include "raytracing.hh"
#include "types.hh"
#include "vectors.hh"

namespace RE
{
    static vec3_t get_baricentric(vec3_t p, vec3_t a, vec3_t b, vec3_t c)
    {
        //Christer Ericson's Real-Time Collision Detection
        vec3_t v0 = b - a;
        vec3_t v1 = c - a;
        vec3_t v2 = p - a;

        float d00 = dot(v0, v0);
        float d01 = dot(v0, v1);
        float d11 = dot(v1, v1);
        float d20 = dot(v2, v0);
        float d21 = dot(v2, v1);
        float denom = d00 * d11 - d01 * d01;

        float v = (d11 * d20 - d01 * d21) / denom;
        float w = (d00 * d21 - d01 * d20) / denom;
        float u = 1.0f - v - w;

        return vec3_t(u, v, w);
    }

    vec3_t get_triangle_uv(vec3_t vtx[3], vec3_t uv[3], vec3_t pt)
    {
        vec3_t b = get_baricentric(pt, vtx[0], vtx[1], vtx[2]);
        return uv[0] * b.x + uv[1] * b.y + uv[2] * b.z;
    }

    vec3_t get_sphere_uv(vec3_t center, vec3_t pt)
    {
        vec3_t vp = normalize(pt - center);
        float phi = acos(-dot(VECTOR_UP, vp));
        float v = phi / M_PI;
        float u = 0;

        float theta = acos(dot(vp, VECTOR_RIGHT) / sin(phi)) / (2.f * M_PI);

        if (dot(cross(VECTOR_UP, VECTOR_RIGHT), vp) > 0)
            u = theta;
        else
            u = 1.f - theta;
        return vec3_t(u, v);
    }

    static vec3_t get_checker_color(vec3_t uv)
    {
        const float scale = 10.0;
        uv *= scale;

        int color = (int)(uv.x) % 2;
        color += (int)(uv.y) % 2;
        color = color & 1;

        return vec3_t(color, color, color);
    }

    vec3_t get_diffuse_color(scene_t *scene, hit_t& hit)
    {
        if (!hit.object->mlt.has_texture)
            return hit.object->mlt.diffuse;

        return get_checker_color(hit.uv_coord);
    }

}
