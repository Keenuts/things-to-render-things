#include <algorithm>
#include <assert.h>
#include <iostream>
#include <mutex>
#include <vector>
#include <thread>
#include <SDL2/SDL.h>
#include <signal.h>

#include "defines.hh"
#include "framework.hh"
#include "lodepng.hh"
#include "types.hh"
#include "viewer.hh"


namespace RE
{
    static void main_loop(struct viewer_state& state)
    {
        while (!*state.should_close && !SDL_QuitRequested()) {
            void *data;
            int pitch;

            SDL_LockTexture(state.texture, NULL, &data, &pitch);

            memcpy(data, state.output_frame, state.width * state.height * STRIDE);

            SDL_UnlockTexture(state.texture);

            SDL_RenderClear(state.renderer);
            SDL_RenderCopy(state.renderer, state.texture, NULL, NULL);
            SDL_RenderPresent(state.renderer);

            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }

        puts("Exiting now");
    }

    struct viewer_state initialize_viewport(struct renderer_info info)
    {
        struct viewer_state state;

        memset(&state, 0, sizeof(state));
        state.mutex = new std::mutex();
        state.should_close = new bool;
        state.width = info.width;
        state.height = info.height;
        state.output_frame = info.output_frame;

        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
        signal(SIGINT, SIG_DFL);
        SDL_CreateWindowAndRenderer(info.width, info.height,
                                    0, &state.window, &state.renderer);

        state.texture = SDL_CreateTexture(state.renderer, SDL_PIXELFORMAT_RGBA32,
                          SDL_TEXTUREACCESS_STREAMING,
                          info.width, info.height);

        state.gui_thread = new std::thread(main_loop, std::ref(state));

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
