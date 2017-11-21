#pragma once

#include <mutex>
#include <thread>

#include <SDL2/SDL.h>

namespace RE
{
    struct viewer_state {
        std::mutex *mutex;

        uint32_t width, height;
        bool *should_close;
        uint8_t *output_frame;

        SDL_Renderer *renderer;
        SDL_Window *window;
        SDL_Texture *texture;
        std::thread *gui_thread;
    };

    struct viewer_state initialize_viewport(struct renderer_info info);
    void destroy_viewer(struct viewer_state& state);
}
