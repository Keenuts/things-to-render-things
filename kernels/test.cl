__kernel void my_memset(global uchar *output)
{
    int id = (get_global_id(0) + get_global_id(1) * 512) * 4;

    output[id + 0] = 255;
    output[id + 1] = 0;
    output[id + 2] = 0;
    output[id + 3] = 252;
}
