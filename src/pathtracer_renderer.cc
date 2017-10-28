#include <stdio.h>
#include <string.h>

#include "lodepng.hh"
#include "pathtracer_renderer.hh"
#include "raytracing.hh"

#define STRIDE 4 //(BGR)

namespace pathtracer
{
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
                vec3_t b = scene->camera_position + scene->camera_direction;
                b.x -= info.width * 0.5f - x;
                b.y -= info.height * 0.5f - y;

                ray_t r;
                r.direction = normalize(b - scene->camera_position);
                r.origin = scene->camera_position;

                for (auto& o : scene->objects) {
                    if (o->type == object_type_e::SPHERE)
                        render_sphere(&info, o, r);
                }

                info.output_frame[(info.width * y + x) * STRIDE + 0] = 0;
                info.output_frame[(info.width * y + x) * STRIDE + 1] = 255;
                info.output_frame[(info.width * y + x) * STRIDE + 2] = 0;
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

    void render_sphere(render_info_t *info, object_t *object, ray_t ray)
    {
    }
}
