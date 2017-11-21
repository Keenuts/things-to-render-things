#include <assert.h>
#include <chrono>
#include <fstream>
#include <queue>
#include <stack>
#include <stdio.h>
#include <thread>

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

    static void worker(struct renderer_info& i, std::queue<uint32_t>& q, std::mutex& m)
    {
        uint32_t width = i.width;

        while (!q.empty()) {
            m.lock();
            if (q.empty()) {
                m.unlock();
                return;
            }

            uint32_t y = q.front();
            q.pop();
            m.unlock();

            for (uint32_t x = 0; x < width; x++) {
                vec3_t px = render_pixel(i, x, y);

                i.output_frame[(x + y * width) * STRIDE + 0] = px.r * 255.0;
                i.output_frame[(x + y * width) * STRIDE + 1] = px.g * 255.0;
                i.output_frame[(x + y * width) * STRIDE + 2] = px.b * 255.0;
                i.output_frame[(x + y * width) * STRIDE + 3] = 255;
            }
        }
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

        std::queue<uint32_t> jobs;
        std::vector<std::thread> threads(0);
        std::mutex lock;

        for (uint32_t y = 0; y < height; y++)
            jobs.push(y);

        for (uint32_t i = 0; i < MAX_THREADS; i++)
            threads.emplace_back(worker, std::ref(info), std::ref(jobs), std::ref(lock));

        for (uint32_t i = 0; i < MAX_THREADS; i++)
            threads[i].join();
        
        lodepng::encode("output.png", info.output_frame, info.width, info.height);
        delete[] info.output_frame;
    }
}
