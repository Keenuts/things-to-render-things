#include <CL/cl.hpp>
#include <assert.h>
#include <fstream>
#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "defines.hh"
#include "opencl_wrapping.hh"
#include "pathtracer_framework.hh"
#include "renderer.hh"
#include "scoped_timer.hh"

namespace pathtracer
{
    static struct kernel_object*
    convert_objects(const std::vector<object_t*>& objects)
    {
        struct kernel_object *list = new struct kernel_object[objects.size()];
        uint64_t i = 0;

        for (object_t *o : objects) {
            list[i].type = o->type;
            list[i].position = to_float3(o->position);
            list[i].rotation = to_float3(o->rotation);
            list[i].color = to_float3(o->color);

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

    static void initialize_opencl(context_t& ctx)
    {
        cl_int res = CL_SUCCESS;

        std::vector<cl::Platform> platforms;
        std::vector<cl::Device> devices;

        res = cl::Platform::get(&platforms);
        assert(res == CL_SUCCESS && platforms.size() != 0);

        cl::Platform p = platforms[0];
        res = p.getDevices(CL_DEVICE_TYPE_ALL, &devices);

        if (res == CL_SUCCESS) {
            assert(devices.size() > 0);
            ctx.device = devices[0];
            ctx.platform = p;
        }
        else
            assert(0 && "Bad platform");

        ctx.opencl_ctx = cl::Context({ ctx.device });
    }

    static void load_pathtracing_kernel(context_t& ctx)
    {
        const char* filename = "../kernels/pathtracing.cl";
        cl_int res;
        std::ifstream f(filename);
        assert(f.is_open() && "Unable to load the kernel");

        std::string content((std::istreambuf_iterator<char>(f)),
                            std::istreambuf_iterator<char>());
        f.close();

        cl::Program k(ctx.opencl_ctx, content);

        res = k.build({ ctx.device }, "-I../kernels");
        if (res != CL_SUCCESS) {
            fprintf(stderr, "[Kernel] Unable to build the kernel : %s\n", filename);
            fprintf(stderr, "%s\n",
                            k.getBuildInfo<CL_PROGRAM_BUILD_LOG>(ctx.device).c_str());
            assert(0);
        }

        ctx.kernel = k;
    }

    static struct kernel_parameters prepare_kernel_data(
        context_t ctx, cl::CommandQueue queue, uint32_t width, uint32_t height)
    {
        scene_t *scene = ctx.scene;
        struct kernel_parameters p;

        cl_mem_flags flags = CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY;
        cl::ImageFormat format(CL_RGBA, CL_UNSIGNED_INT8);
        cl_int res;

        cl::Image2D image(ctx.opencl_ctx, flags, format, width, height, 0, NULL, &res);
        assert(res == CL_SUCCESS);
        p.image = image;

        uint64_t obj_count = scene->objects.size();
        uint64_t vtx_count = 16;

        uint64_t obj_buf_size = sizeof(struct kernel_object) * obj_count;
        uint64_t vtx_buf_size = sizeof(cl_double3) * vtx_count;

        struct kernel_object *k_objects = convert_objects(scene->objects);
        cl_double3 *k_vertex = pack_vertex(scene->objects);

        cl::Buffer obj_buf(ctx.opencl_ctx, CL_MEM_READ_ONLY, obj_buf_size);
        cl::Buffer vtx_buf(ctx.opencl_ctx, CL_MEM_READ_ONLY, vtx_buf_size);

        p.vertex  = vtx_buf;
        p.objects = obj_buf;

        queue.enqueueWriteBuffer(p.objects, CL_TRUE, 0, obj_buf_size, k_objects);
        queue.enqueueWriteBuffer(p.vertex, CL_TRUE, 0, vtx_buf_size, k_vertex);

        delete[] k_objects;
        delete[] k_vertex;

        p.info.samples = SAMPLES_COUNT;
        p.info.depth = MAX_DEPTH;
        p.info.block_width  = PX_PER_THREAD_X;
        p.info.block_height = PX_PER_THREAD_Y;
        p.info.width = width;
        p.info.height = height;
        p.info.camera_position = to_float3(scene->camera_position);
        p.info.camera_direction = to_float3(scene->camera_direction);
        p.info.camera_up = to_float3(VECTOR_UP);
        p.info.camera_right = to_float3(VECTOR_RIGHT);
        p.info.object_count = obj_count;
        p.info.vtx_count = vtx_count;

        return p;
    }

    static void show_time(bool *finished)
    {
        double elapsed = 0.0;
        while (!*finished) {
            double l;
            {
                scoped_timer t(l);
                printf("Elapsed time: %d:%0.2ds  \n\b\r", (int)(elapsed / 60.0),
                (int)elapsed % 60);
                sleep(1);
            }
            elapsed += l;
        }
        printf("done\n.");
    }

    void pathtracer_gpu(context_t& ctx, uint32_t width, uint32_t height)
    {
        initialize_opencl(ctx);
        load_pathtracing_kernel(ctx);

        cl::CommandQueue queue(ctx.opencl_ctx, ctx.device);
        struct kernel_parameters p = prepare_kernel_data(ctx, queue, width, height);

        cl::Kernel k = cl::Kernel(ctx.kernel,"pathtracer");
        cl_int res;
        res = k.setArg(0, p.info);
        assert(res == CL_SUCCESS);
        res = k.setArg(1, p.objects);
        assert(res == CL_SUCCESS);
        res = k.setArg(2, p.vertex);
        assert(res == CL_SUCCESS);
        res = k.setArg(3, p.image);
        assert(res == CL_SUCCESS);


        uint32_t gdim_x = width / PX_PER_THREAD_X;
        uint32_t gdim_y = height / PX_PER_THREAD_Y;

        uint32_t dim_x = 32;
        uint32_t dim_y = 32;

        printf("Opencl Dispatch: [%ux%u] [%ux%u].\n", gdim_x, gdim_y, dim_x, dim_y);

        double elapsed;
        {
            scoped_timer t(elapsed);
            res = queue.enqueueNDRangeKernel(k,
                cl::NullRange,
                cl::NDRange(gdim_x, gdim_y),
                cl::NDRange(dim_x, dim_y)
            );
            assert(res == CL_SUCCESS);

            bool finished = false;
            std::thread timer(show_time, &finished);
            queue.finish();

            finished = true;
            timer.join();
        }

        cl::size_t<3> origin, region;
        origin[0] = 0; origin[1] = 0; origin[2] = 0;
        region[0] = width; region[1] = height; region[2] = 1;

        queue.enqueueReadImage(
            p.image,
            CL_TRUE,
            origin,
            region,
            0,
            0,
            ctx.output_frame,
            nullptr,
            nullptr
        );
        printf("Rendering done in %.2lf seconds.\n", elapsed);
    }
}
