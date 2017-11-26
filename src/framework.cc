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
#if defined(USE_MDT)
        return mdt(i.scene, r);
#elif defined(USE_RAYTRACER)
        return raytrace(i.scene, r, 0);
#elif defined(USE_PATHTRACER)
        {
            vec3_t out = BLACK;
            for (uint32_t s = 0; s < PT_SAMPLES; s++)
                out = out + pathtrace(i.scene, r, 0) * (1.0f / PT_SAMPLES);
            return saturate(out);
        }
#else
    #error "No rendering method selected"
#endif
    }

    struct job {
        uint32_t x, y, width;
    };

    static void worker(struct renderer_info& i, std::queue<struct job>& q, std::mutex& m)
    {
        while (!q.empty()) {
            m.lock();
            if (q.empty()) {
                m.unlock();
                return;
            }

            struct job j = q.front();
            q.pop();
            m.unlock();

            uint32_t lim = std::min(i.width, j.x + j.width);
            for (uint32_t x = j.x; x < lim; x++) {
                vec3_t px = render_pixel(i, x, j.y);

                i.output_frame[(x + j.y * i.width) * STRIDE + 0] = px.r * 255.0;
                i.output_frame[(x + j.y * i.width) * STRIDE + 1] = px.g * 255.0;
                i.output_frame[(x + j.y * i.width) * STRIDE + 2] = px.b * 255.0;
                i.output_frame[(x + j.y * i.width) * STRIDE + 3] = 255;
            }
        }
    }

    void render_scene(scene_t *scene, uint32_t width, uint32_t height,
                      struct area *area)
    {
        struct renderer_info info;
        struct viewer_state viewer_state;
        memset(&info, 0, sizeof(info));

        info.width = width;
        info.height = height;
        info.output_frame = new uint8_t[width * height * STRIDE];
        info.scene = scene;

        viewer_state = initialize_viewport(info);

#if defined(USE_MDT)
        mdt_generate_irradiance_lights(scene);
#endif

        std::queue<struct job> jobs;
        std::vector<std::thread> threads(0);
        std::mutex lock;

        uint32_t x = area ? area->x : 0;
        uint32_t w = area ? area->w : width;
        for (uint32_t y = 0; y < height; y++) {
            struct job j = { x, y, w };
            if (!area || (area->y <= y && area->y + area->h > y))
                jobs.push(j);
        }

        for (uint32_t i = 0; i < MAX_THREADS; i++)
            threads.emplace_back(worker, std::ref(info), std::ref(jobs), std::ref(lock));

        for (uint32_t i = 0; i < MAX_THREADS; i++)
            threads[i].join();
        

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        lodepng::encode("output.png", info.output_frame, info.width, info.height);
        puts("Output written to the disk");
        delete[] info.output_frame;
    }
}
