#include <assert.h>
#include <chrono>
#include <fstream>
#include <stack>
#include <stdio.h>
#include <thread>
#include <CL/cl.h>

#include "defines.hh"
#include "framework.hh"
#include "lodepng.hh"
#include "renderer.hh"
#include "scoped_timer.hh"
#include "viewer.hh"

namespace RE
{
    static vec3_t render_pixel(struct renderer_info& i, uint32_t x, uint32_t y)
    {
        ray_t r = get_ray_from_camera(i, x, y);
        return mdt(i.scene, r, i.scene->mdt_lights, i.scene->mdt_light_count);
    }

    void render_scene(scene_t *scene, uint32_t width, uint32_t height)
    {
        struct renderer_info info;
        struct viewer_state viewer_state;
        memset(&info, 0, sizeof(info));

        info.width = width;
        info.height = height;
        info.output_frame = new uint8_t[width * height * STRIDE];
        info.scene = scene;

        viewer_state = initialize_viewport(info);

        scene->mdt_lights = mdt_generate_irradiance_lights(scene, &scene->mdt_light_count);

        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                vec3_t px = render_pixel(info, x, y);

                info.output_frame[(x + y * width) * STRIDE + 0] = px.r * 255.0;
                info.output_frame[(x + y * width) * STRIDE + 1] = px.g * 255.0;
                info.output_frame[(x + y * width) * STRIDE + 2] = px.b * 255.0;
                info.output_frame[(x + y * width) * STRIDE + 3] = 255;
            }

            float percent = ((float)y / height) * 100.0f;
            printf("Percent %.2f\n", percent);
        }

        lodepng::encode("output.png", info.output_frame, info.width, info.height);
        delete[] info.output_frame;
    }
}
