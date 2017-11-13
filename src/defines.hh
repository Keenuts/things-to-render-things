#pragma once

//#define USE_RAYTRACER
#define USE_OPENCL

#define WIDTH 512
#define HEIGHT 512

// For OpenCL
#define PX_PER_THREAD_X 1
#define PX_PER_THREAD_Y 1


// For CPU Rendering
#define MAX_THREADS_X 2
#define MAX_THREADS_Y 2
#define SAMPLES_COUNT 3
#define SAMPLE_DEPTH 1


// Raytracer settings
#define SOFT_SHADOW_SAMPLES 4
#define SOFT_SHADOW_RADIUS 0.005
