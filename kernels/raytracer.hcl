static float3 raytracer_render_ray(scene_t *scene, ray_t ray, int max_depth)
{
    float3 color = BLACK;

    hit_t hit;
    if (!intersect_scene(scene, ray, &hit))
        return BLACK;

    if (hit.object.type == AREA_LIGHT)
        return hit.object.mlt.emissive;


    //float3 sample_dir = get_hemisphere_random(scene, hit.normal);
    float3 sample_dir = hit.normal;
    ray_t light_ray;
    light_ray.origin = hit.position + hit.normal * F_EPSYLON;
    light_ray.direction = sample_dir;
    hit_t light_hit = hit;

    if (intersect_scene(scene, light_ray, &light_hit))
    {
        if (light_hit.object.type == AREA_LIGHT) {
            float3 v = normalize(hit.position - light_hit.position);
            float factor = max(0.f, dot(hit.normal, v));
            color = light_hit.object.mlt.emissive;
        }
        else
            color = hit.object.mlt.diffuse;
    }

    return color;
}
