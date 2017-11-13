#pragma once

//#define RAYTRACING
#define USE_OPENCL

#define WIDTH 512
#define HEIGHT 512

#define SAMPLES_COUNT 3
#define SAMPLE_DEPTH 1


// For OpenCL
#define PX_PER_THREAD_X 1
#define PX_PER_THREAD_Y 1


// For CPU Rendering
#define MAX_THREADS_X 2
#define MAX_THREADS_Y 2
