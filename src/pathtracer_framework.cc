#include <stack>
#include <stdio.h>
#include <thread>

#include "defines.hh"
#include "lodepng.hh"
#include "pathtracer_framework.hh"
#include "pathtracer_renderer.hh"
#include "scoped_timer.hh"

#define STRIDE 4 //(RGBA)

namespace pathtracer
{
    void render_scene(scene_t *scene, uint32_t width, uint32_t height)
    {
        context_t ctx;

        ctx.width = width;
        ctx.height = height;
        ctx.output_frame = new uint8_t[ctx.width * ctx.height * STRIDE];
        ctx.scene = scene;

        uint32_t block_width = height / MAX_THREADS_X;
        uint32_t block_height = width / MAX_THREADS_Y;
        printf("Starting %u threads working on blocks [%ux%u] px.\n",
                MAX_THREADS_X * MAX_THREADS_Y, block_width, block_height);

        std::stack<std::thread> workers;

        for (uint32_t y = 0; y < sqrt(MAX_THREADS_Y); y++) {
            for (uint32_t x = 0; x < sqrt(MAX_THREADS_X); x ++) {
                workers.emplace(
                    render_scene_chunk,
                    ctx,
                    vec3_t(block_width, block_height),
                    vec3_t(x, y, 0)
                );
            }
        }

        printf("Waiting for threads to finish.\n");

        double elapsed;
        {
            scoped_timer_t timer(elapsed);
            while (workers.size() > 0) {
                workers.top().join();
                printf("Worker done.\n");
                workers.pop();
            }
        }

        printf("Rendering done in %.2lf seconds.\n", elapsed);

        lodepng::encode("output.png", ctx.output_frame, ctx.width, ctx.height);
        printf("Output written to disk.\n");

        delete[] ctx.output_frame;
        printf("Memory freed.\n");
    }

    void render_scene_chunk(context_t ctx, vec3_t size, vec3_t block_pos)
    {
        uint32_t start_x = block_pos.x * size.x;
        uint32_t start_y = block_pos.y * size.y;
        uint32_t size_x = size.x;
        uint32_t size_y = size.y;

        uint8_t *out = new uint8_t[size_x * size_y * STRIDE];

        for (uint32_t y = 0; y < size_y; y++) {
            for (uint32_t x = 0; x < size_x; x++) {

                ray_t r = get_ray_from_camera(ctx, start_x + x, start_y + y);
                vec3_t color = render_ray(ctx.scene, r, 0);

                out[(size_x * y + x) * STRIDE + 0] = color.r * 255.0;
                out[(size_x * y + x) * STRIDE + 1] = color.g * 255.0;
                out[(size_x * y + x) * STRIDE + 2] = color.b * 255.0;
                out[(size_x * y + x) * STRIDE + 3] = 255;
            }
        }

        for (uint32_t y = 0; y < size_y; y++) {
            uint32_t offset = (start_y + y) * ctx.width + start_x;
            memcpy(ctx.output_frame + offset * STRIDE, out + y * size_x * STRIDE, size_x * STRIDE); 
        }

        delete[] out;
    }

}
