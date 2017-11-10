#include <stdio.h>
#include <string.h>
#include <limits>
#include <cassert>

#include "lodepng.hh"
#include "pathtracer_renderer.hh"
#include "raytracing.hh"
#include "helpers.hh"

#define STRIDE 4 //(BGR)
#define PI 3.14159265358979323846

namespace pathtracer
{
    static ray_t create_ray_from_px(uint32_t x, uint32_t y, scene_t *scene)
    {
        double width = scene->width;
        double height = scene->height;
        const double FOV = 45.0;
        const double DEG2RAD = (PI * 2.0) / 360.0;

        double L = width / (tan(DEG2RAD * (FOV / 2.0)) * 2.0);

        vec3_t middle = scene->camera_position + scene->camera_direction * L;
        double s_width = lerp(-width, width, x / width) / 2.0;
        double s_height = lerp(-height, height, y / height) / 2.0;

        vec3_t screen_ptr = VECTOR_RIGHT * s_width
                      + VECTOR_UP * s_height
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

                info.output_frame[(info.width * y + x) * STRIDE + 0] = color.r;
                info.output_frame[(info.width * y + x) * STRIDE + 1] = color.g;
                info.output_frame[(info.width * y + x) * STRIDE + 2] = color.b;
                info.output_frame[(info.width * y + x) * STRIDE + 3] = 255;
            }
        }

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
        vec3_t px(0, 0, 0);

        for (object_t *o : scene->objects) {
            double c_depth;
            vec3_t c_px;

            if (o->type == object_type_e::SPHERE)
                c_px = render_object(scene,static_cast<object_sphere_t*>(o),
                    ray, &c_depth, bounce);
            else
                assert(0);

            if (c_depth < depth) {
                px = c_px;
                depth = c_depth;
            }
        }

        return px;
    }

    vec3_t render_object(scene_t *scene, object_sphere_t *o, ray_t r,
                         double *depth, uint32_t bounce)
    {
        vec3_t intersection;

        if (intersect_sphere(r, o->position, o->radius, &intersection)) {
            *depth = magnitude(intersection - r.origin);

            vec3_t normal = normalize(intersection - o->position);
            vec3_t light = normalize(vec3_t(-0.2, -1, -0.2));
            vec3_t color = std::max(0.1, dot(normal, light)) * o->color;
            return color;
        }


        *depth = std::numeric_limits<double>::infinity();
        return vec3_t(0, 0, 0);
    }

}
