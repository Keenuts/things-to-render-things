#include <assert.h>
#include <chrono>
#include <fstream>
#include <stack>
#include <stdio.h>
#include <thread>

#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX

#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "defines.hh"
#include "lodepng.hh"
#include "framework.hh"
#include "viewer.hh"
#include "renderer.hh"
#include "scoped_timer.hh"

namespace RE
{
    static void initialize_opencl(struct renderer_info& i)
    {
        cl_int res;
        cl_platform_id platforms[2];
        cl_device_id devices[2];
        cl_uint platform_count, device_count;

        res = clGetPlatformIDs(2, platforms, &platform_count);
        assert(res == CL_SUCCESS && platform_count > 0);

        res = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 2, devices, &device_count);
        assert(res == CL_SUCCESS && device_count > 0);
        i.device = devices[0];

        cl_context_properties properties[] = {
            CL_GL_CONTEXT_KHR, (cl_context_properties)glfwGetCurrentContext(),
            CL_GLX_DISPLAY_KHR, (cl_context_properties)glfwGetX11Display(),
            CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[0],
            0
        };

        i.context = clCreateContext(properties, 1, devices, nullptr, nullptr, &res);
        assert(res == CL_SUCCESS);

        cl_command_queue_properties cq_props[] = { 0 };
        i.queue = clCreateCommandQueueWithProperties(i.context, devices[0], cq_props, &res);
        assert(res == CL_SUCCESS);
    }

    static bool load_kernel(struct renderer_info& i, const char* filename, const char* ker)
    {
        cl_int res;
        cl_program p;

        std::ifstream f(filename);
        if (!f.is_open()) {
            fprintf(stderr, "Unable to open source file '%s'.\n", filename);
            return false;
        }

        std::string content((std::istreambuf_iterator<char>(f)),
                            std::istreambuf_iterator<char>());
        f.close();

        const char *src = content.c_str();
        uint64_t src_length = content.length();
        p = clCreateProgramWithSource(i.context, 1, &src, &src_length, &res);	
        if (res != CL_SUCCESS)
            return false;

        puts("Building kernel...");
        res = clBuildProgram(p, 1, &i.device, "-I../kernels", nullptr, nullptr);
        if (res != CL_SUCCESS) {

            char *output = new char[2048];
            uint64_t output_len;
            output[0] = 0;

            clGetProgramBuildInfo(p, i.device, CL_PROGRAM_BUILD_LOG,
                                   2048, output, &output_len);
            fprintf(stderr, "Failed to build the kernel [%d]:\n%s\n", res, output);
            return false;
        }

        i.kernel = clCreateKernel(p, ker, &res);
        if (res != CL_SUCCESS) {
            fprintf(stderr, "Cannot find the kernel %s\n", ker);
            return false;
        }

        return true;
    }

    template<typename T>
    static inline T align(T value, T divisor)
    {
        return (~(divisor - 1) & (value - 1)) + divisor;
    }

    static struct kernel_object*
    convert_objects(const std::vector<object_t*>& objects)
    {
        struct kernel_object *list = new struct kernel_object[objects.size()];
        uint64_t i = 0;

        for (object_t *o : objects) {
            list[i].type = o->type;
            list[i].position = to_float3(o->position);
            list[i].rotation = to_float3(o->rotation);
            list[i].mlt.diffuse = to_float3(o->mlt.diffuse);
            list[i].mlt.emission = to_float3(o->mlt.emission);

            switch(o->type) {
                case object_type_e::SPHERE:
                    list[i].radius = static_cast<object_sphere_t*>(o)->radius;
                    break;
                case object_type_e::PLANE:
                    list[i].normal = to_float3(static_cast<object_plane_t*>(o)->normal);
                    break;
                case object_type_e::AREA_LIGHT:
                    list[i].normal = to_float3(static_cast<area_light_t*>(o)->normal);
                    list[i].size = to_float3(static_cast<area_light_t*>(o)->size);
                    break;
                default:
                    assert(0 && "Unhandled object type.");
            };
            i++;
        }

        return list;
    }

    static cl_double3*
    pack_vertex(const std::vector<object_t*>& objects)
    {
        cl_double3 *vertex = new cl_double3[16];
        return vertex;
    }


    static struct kernel_parameters prepare_kernel_data(struct renderer_info& i, scene_t *s,
                             uint64_t pos[2], uint64_t size[2])
    {
        cl_int res;
        struct kernel_parameters p;

        uint64_t obj_count = s->objects.size();
        uint64_t vtx_count = 16;
        uint64_t obj_buf_size = sizeof(struct kernel_object) * obj_count;
        uint64_t vtx_buf_size = sizeof(cl_double3) * vtx_count;

        struct kernel_object *k_objects = convert_objects(s->objects);
        cl_double3 *k_vertex = pack_vertex(s->objects);

        cl_mem_flags flags = CL_MEM_READ_ONLY;

        cl_mem obj_mem = clCreateBuffer(i.context, flags, obj_buf_size, 0, &res);
        assert(res == CL_SUCCESS);
        cl_mem vtx_mem = clCreateBuffer(i.context, flags, vtx_buf_size, 0, &res);
        assert(res == CL_SUCCESS);

        p.vertex  = vtx_mem;
        p.objects = obj_mem;

        clEnqueueWriteBuffer(i.queue, obj_mem, true, 0, obj_buf_size, k_objects, 0, 0, 0);
        clEnqueueWriteBuffer(i.queue, vtx_mem, true, 0, vtx_buf_size, k_vertex, 0, 0, 0);

        delete[] k_objects;
        delete[] k_vertex;

        p.info.samples = SAMPLES_COUNT;
        p.info.depth = MAX_DEPTH;
        p.info.offset_x = pos[0];
        p.info.offset_y = pos[1];
        p.info.stride = STRIDE;
        p.info.width = i.width;
        p.info.height = i.height;

        p.info.camera_position = to_float3(s->camera_position);
        p.info.camera_direction = to_float3(s->camera_direction);
        p.info.camera_up = to_float3(VECTOR_UP);
        p.info.camera_right = to_float3(VECTOR_RIGHT);
        p.info.object_count = obj_count;
        p.info.vtx_count = vtx_count;

        return p;
    }

    static void render_block(struct renderer_info& i, scene_t *s,
                             uint64_t pos[2], uint64_t size[2])
    {
        cl_int res;


        struct kernel_parameters p = prepare_kernel_data(i, s, pos, size);

        res = clSetKernelArg(i.kernel, 0, sizeof(i.mem), &i.mem);
        assert(res == CL_SUCCESS);
        res = clSetKernelArg(i.kernel, 2, sizeof(p.objects), &p.objects);
        assert(res == CL_SUCCESS);
        res = clSetKernelArg(i.kernel, 3, sizeof(p.vertex), &p.vertex);
        assert(res == CL_SUCCESS);

        for (uint32_t line = 0; line < size[1]; line++) {
            size_t global_size[2] = {
                std::min(align(size[0], 16UL), 512UL),
                1
            };

            res = clSetKernelArg(i.kernel, 1, sizeof(p.info), &p.info);
            assert(res == CL_SUCCESS);

            size_t local_size[2] = { 16, 1 };

            res = clEnqueueNDRangeKernel(i.queue, i.kernel,
                    2, nullptr,
                    global_size, local_size,
                    0, nullptr, nullptr);
            if (res)
                printf("CL-error: %d\n", res);
            assert(res == CL_SUCCESS);
            p.info.offset_y++;
        }

        clFlush(i.queue);

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

        initialize_opencl(info); 
        if (!load_kernel(info, KERNEL_PATH, KERNEL_NAME))
            exit(2);
        puts("Starting rendering...");

        viewer_state = initialize_viewport(info);
        info.mem = viewer_state.buffer;

#define NBR_LINES 32

        for (uint32_t i = 0; i < NBR_LINES; i++) {
            uint32_t block_height = (info.height / NBR_LINES);
            uint32_t y = i * block_height;

            uint64_t p[2] = { 0, y };
            uint64_t s[2] = { info.width, block_height};

            viewer_state.mutex->lock();

            glfwMakeContextCurrent(viewer_state.window);
            render_block(info, scene, p, s);
            viewer_state.mutex->unlock();
            clFinish(info.queue);

            struct update_area area = {
                y * info.width * STRIDE,            // Start
                info.width * block_height * STRIDE,    // Size
                0, (uint32_t)y,                     // dst X, Y
                info.width,                         // dst width
                info.height / NBR_LINES             // dst height
            };

            refresh_chunk(viewer_state, area);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        destroy_viewer(viewer_state);

        clEnqueueReadBuffer(info.queue, info.mem, true, 0,
                            info.width * info.height * STRIDE,
                            info.output_frame, 0, 0, 0);
        lodepng::encode("output.png", info.output_frame, info.width, info.height);
        delete[] info.output_frame;
    }
}
