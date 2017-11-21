#pragma once

#include <mutex>
#include <thread>

#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace RE
{
    struct viewer_state {
        uint32_t width, height;
        std::mutex *mutex;

        cl_mem buffer;
        cl_command_queue queue;
        GLuint texture_id, buffer_id;
        void *host_ptr;

        GLFWwindow *window;
        bool *terminate;
        std::thread *gui_thread;
    };

    struct update_area {
        uint64_t src_start;
        uint64_t src_size;
        uint32_t dst_x, dst_y;
        uint64_t dst_w, dst_h;
    };

    struct viewer_state initialize_viewport(struct renderer_info info);
    void refresh_chunk(struct viewer_state& state, struct update_area area);
    void destroy_viewer(struct viewer_state& state);
}
