#include <assert.h>
#include <algorithm>
#include <CL/cl.h>
#include <CL/cl_gl.h>

#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "viewer.hh"
#include "types.hh"
#include "lodepng.hh"

namespace RE
{
    struct cl_data {
        cl_context ctx;
        cl_command_queue queue;
        cl_kernel kernel;
        cl_device_id device;
    };

    static void cb_on_error(const char *info, const void *p, uint64_t cb, void *u)
    {
        puts(info);
    }

    static struct cl_data initialize_opencl()
    {
        struct cl_data i;
        cl_int res;
        cl_platform_id platforms[10];
        cl_device_id devices[10];
        cl_uint platform_count, device_count;

        res = clGetPlatformIDs(10, platforms, &platform_count);
        assert(res == CL_SUCCESS && platform_count > 0);

        res = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 10, devices, &device_count);
        assert(res == CL_SUCCESS && device_count > 0);
        i.device = devices[0];


        cl_context_properties properties[] = {
            CL_GL_CONTEXT_KHR, (cl_context_properties)glfwGetCurrentContext(),
            CL_GLX_DISPLAY_KHR, (cl_context_properties)glfwGetX11Display(),
            CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[0],
            0
        };


        i.ctx = clCreateContext(properties, 1, devices, cb_on_error, nullptr, &res);
        assert(res == CL_SUCCESS);

        cl_command_queue_properties cq_props[] = { 0 };
        i.queue = clCreateCommandQueueWithProperties(i.ctx, devices[0], cq_props, &res);
        assert(res == CL_SUCCESS);

        return i;
    }

    static void display(GLFWwindow *window, GLuint tex)
    {
        float ratio;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, tex);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glEnable(GL_TEXTURE_2D);

        glBegin(GL_QUADS);
            glTexCoord2i(0, 0);
            glVertex3f(-1.f, -1.f, 0.f);

            glTexCoord2i(0, 1);
            glVertex3f(-1.f, 1.f, 0.f);

            glTexCoord2i(1, 1);
            glVertex3f(1.f, 1.f, 0.f);

            glTexCoord2i(1, 0);
            glVertex3f(1.f, -1.f, 0.f);
        glEnd();

        glDisable(GL_TEXTURE_2D);

        glfwSwapBuffers(window);

    }

    static void load_kernel(struct cl_data& i)
    {
        cl_int res;
        cl_program p;
#define MAX_SOURCE_SIZE 1024 * 1024

        FILE *f = fopen("../kernels/test.cl", "r");
        if (!f) {
            fprintf(stderr, "Failed to load kernel.\n");
            exit(1);
        }

        char *source_str = (char *)malloc(MAX_SOURCE_SIZE);
        uint64_t source_size = fread(source_str, 1, MAX_SOURCE_SIZE, f);
        fclose(f);

        p = clCreateProgramWithSource(i.ctx, 1, (const char **)&source_str,
                                      (const size_t *)&source_size, &res);	
        assert(res == CL_SUCCESS);

        res = clBuildProgram(p, 1, &i.device, nullptr, nullptr, nullptr);
        if (res != CL_SUCCESS) {

            char *output = new char[2048];
            uint64_t output_len;
            clGetProgramBuildInfo(p, i.device, CL_PROGRAM_BUILD_LOG,
                                   2048, output, &output_len);
            puts(output);
        }

        assert(res == CL_SUCCESS);

        i.kernel = clCreateKernel(p, "my_memset", &res);
        assert(res == CL_SUCCESS);
    }

    void initialize_viewport()
    {
        GLuint tex, pbo;
        GLFWwindow *window;

        assert(glfwInit());

        window = glfwCreateWindow(512, 512, "Viewer", nullptr, nullptr);
        assert(window);
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        assert(glewInit() == GLEW_OK);


        struct cl_data i = initialize_opencl();
        load_kernel(i);

        cl_int err = 0;

#define BUFFER_SIZE (512 * 512 * 4)

        cl_mem_flags flags = CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR;

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glGenBuffers(1, &pbo);
        glBindBuffer(GL_ARRAY_BUFFER, pbo);
        glBufferData(GL_ARRAY_BUFFER, BUFFER_SIZE, nullptr, GL_DYNAMIC_DRAW);

        void* p = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
        printf("GL error: 0x%x\n", glGetError());
        assert(p != nullptr);

        cl_mem mem = clCreateBuffer(i.ctx, flags, BUFFER_SIZE, p, &err);
        printf("CL error: %d\n", err);
        assert(err == CL_SUCCESS);

        err = clSetKernelArg(i.kernel, 0, sizeof(mem), &mem);
        printf("CL-error: %d\n", err);

        size_t global_size[2] = { 512, 512 };
        size_t local_size[2] = { 16, 16 };

        err = clEnqueueNDRangeKernel(i.queue, i.kernel, 2, nullptr,
                                     global_size, local_size,
                                     0, nullptr, nullptr);
        printf("CL-error: %d\n", err);
        assert(err == CL_SUCCESS);

        clFlush(i.queue);
        clFinish(i.queue);

        //glUnmapBuffer(GL_ARRAY_BUFFER);
        printf("GL Unmap: 0x%x\n", glGetError());

        clEnqueueMapBuffer(i.queue, mem, true, CL_MAP_READ, 0, BUFFER_SIZE, 0, 0, 0, &err);
        printf("CL-error: %d\n", err);
        assert(err == CL_SUCCESS);

        glBindBuffer(GL_ARRAY_BUFFER, pbo);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 512, 512, GL_RGBA, GL_UNSIGNED_BYTE, p);
        printf("GL error: 0x%x\n", glGetError());

        err = clEnqueueUnmapMemObject(i.queue, mem, p, 0, 0, 0);
        printf("CL-error: %d\n", err);
        assert(err == CL_SUCCESS);

        clReleaseMemObject(mem);

        //while (!glfwWindowShouldClose(window))
        for (uint32_t i = 0; i < 30; i++)
            display(window, tex);

        uint8_t *img = new uint8_t[BUFFER_SIZE];
        err = clEnqueueReadBuffer(i.queue, mem, true, 0, BUFFER_SIZE, img, 0, 0, 0);
        assert(err == CL_SUCCESS);
        lodepng::encode("output.png", img, 512, 512);
        puts("Output written to disk.");

        (void)display;
    }
}
