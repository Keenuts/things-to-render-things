#include <algorithm>
#include <assert.h>
#include <iostream>
#include <mutex>
#include <vector>
#include <thread>

#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX

#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>


#include "defines.hh"
#include "framework.hh"
#include "lodepng.hh"
#include "types.hh"
#include "viewer.hh"


namespace RE
{
    static void render_window(struct viewer_state& state)
    {
        float ratio, render_ratio;
        int width, height;

        state.mutex->lock();
        glfwMakeContextCurrent(state.window);

        glfwGetFramebufferSize(state.window, &width, &height);
        ratio = width / (float) height;
        render_ratio = state.width / state.height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D, state.texture_id);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1.0f, 1.0f, -1.f, 1.f, 1.f, -1.f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        float wmul = 1.0f;
        float hmul = 1.0f;

        if (width < height)
            hmul = render_ratio * ratio;
        if (width > height)
            wmul = render_ratio / ratio;

        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
            glTexCoord2i(0, 1);
            glVertex3f(-1.0f * wmul, -1.0f * hmul, 0.f);

            glTexCoord2i(0, 0);
            glVertex3f(-1.0f * wmul, 1.0f * hmul, 0.f);

            glTexCoord2i(1, 0);
            glVertex3f(1.0f * wmul,  1.0f * hmul, 0.f);

            glTexCoord2i(1, 1);
            glVertex3f(1.0f * wmul, -1.0f * hmul, 0.f);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        state.mutex->unlock();
        glfwSwapBuffers(state.window);
    }

    static void main_loop(struct viewer_state state)
    {
        while (!glfwWindowShouldClose(state.window)) {
            render_window(state);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        puts("Exiting now");
    }

    struct viewer_state initialize_viewport(struct renderer_info info)
    {
        struct viewer_state state;
        memset(&state, 0, sizeof(state));

        cl_int err;
        cl_mem_flags flags = CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR;
        uint64_t buffer_size = info.width * info.height * STRIDE;

        state.mutex = new std::mutex();
        state.queue = info.queue;
        state.width = info.width;
        state.height = info.height;

        assert(glfwInit());

        state.window = glfwCreateWindow(512, 512, "TRT", nullptr, nullptr);
        assert(state.window);
        glfwMakeContextCurrent(state.window);
        glfwSwapInterval(1);

        assert(glewInit() == GLEW_OK);

        glGenTextures(1, &state.texture_id);
        glBindTexture(GL_TEXTURE_2D, state.texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     info.width, info.height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glGenBuffers(1, &state.buffer_id);
        glBindBuffer(GL_ARRAY_BUFFER, state.buffer_id);
        glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);

        state.host_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
        assert(state.host_ptr != nullptr);

        state.buffer = clCreateBuffer(info.context, flags, buffer_size,
                                      state.host_ptr, &err);
        assert(err == CL_SUCCESS);

        clFlush(info.queue);
        clFinish(info.queue);
        state.gui_thread = new std::thread(main_loop, state);

        return state;
    }

    void refresh_chunk(struct viewer_state& state, struct update_area area)
    {
        cl_int err;

        state.mutex->lock();
        glfwMakeContextCurrent(state.window);

        state.host_ptr = clEnqueueMapBuffer(state.queue, state.buffer,
                           true, CL_MAP_READ, area.src_start, area.src_size,
                           0, 0, 0, &err);
        assert(err == CL_SUCCESS);

        glBindBuffer(GL_ARRAY_BUFFER, state.buffer_id);
        glBindTexture(GL_TEXTURE_2D, state.texture_id);

        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        area.dst_x, area.dst_y,
                        area.dst_w, area.dst_h,
                        GL_RGBA, GL_UNSIGNED_BYTE,
                        state.host_ptr);

        err = clEnqueueUnmapMemObject(state.queue, state.buffer, state.host_ptr, 0, 0, 0);
        assert(err == CL_SUCCESS);

        clFlush(state.queue);
        clFinish(state.queue);

        state.mutex->unlock();
    }

    void destroy_viewer(struct viewer_state& state)
    {
        state.mutex->lock();
        glfwSetWindowShouldClose(state.window, true);
        state.mutex->unlock();

        state.gui_thread->join();

        glfwDestroyWindow(state.window);
        glfwTerminate();
        clReleaseMemObject(state.buffer);
    }
}
