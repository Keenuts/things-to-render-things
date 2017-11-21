#include <algorithm>
#include <assert.h>
#include <iostream>
#include <mutex>
#include <vector>
#include <thread>
#include <SDL2/SDL.h>

#include "defines.hh"
#include "framework.hh"
#include "lodepng.hh"
#include "types.hh"
#include "viewer.hh"


namespace RE
{
    static void main_loop(struct viewer_state state)
    {
        while (!*state.should_close) {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }

        puts("Exiting now");
    }

    struct viewer_state initialize_viewport(struct renderer_info info)
    {
        SDL_Renderer *renderer;
        SDL_Window *window;
        struct viewer_state state;

        memset(&state, 0, sizeof(state));
        state.mutex = new std::mutex();
        state.should_close = new bool;
        state.width = info.width;
        state.height = info.height;
        state.image = info.output_frame;

        SDL_Init(SDL_INIT_VIDEO);
        SDL_CreateWindowAndRenderer(512, 512, 0, &window, &renderer);
        state.gui_thread = new std::thread(main_loop, state);

        return state;
    }

    void destroy_viewer(struct viewer_state& state)
    {
        state.mutex->lock();
            *state.should_close = true;
        state.mutex->unlock();

        state.gui_thread->join();

        delete state.mutex;
        delete state.should_close;
        delete state.gui_thread;

        SDL_DestroyRenderer(state.renderer);
        SDL_DestroyWindow(state.window);
        SDL_Quit();
    }
}
