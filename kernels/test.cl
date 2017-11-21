#include "types.hcl"

__kernel void my_memset(
    global uchar *output, 
    struct kernel_info info,
    __global object_t *objects,
    __global float3 *vertex
)
{
    int x = info.offset_x + get_global_id(0);
    int y = info.offset_y + get_global_id(1);

    int id = (x + y * info.width) * info.stride;

    output[id + 0] = (x / (float)info.width) * 255.0;
    output[id + 1] = (y / (float)info.height) * 255.0;
    output[id + 2] = 0;
    output[id + 3] = 255;
}
