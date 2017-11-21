#pragma once

//#define USE_RAYTRACER
//#define USE_PATHTRACER
#define USE_MDT
//#define USE_OPENCL

#define WIDTH 512
#define HEIGHT 512

// For CPU Rendering
#define MAX_THREADS_X 2
#define MAX_THREADS_Y 2

// Pathtracer setings
#define SAMPLES_COUNT 2048
#define MAX_DEPTH 3

// Raytracer settings
#define SOFT_SHADOW_SAMPLES 2
#define SOFT_SHADOW_RADIUS 0.005

// MDT settings
#define IR_RAY_PER_LIGHT 16
#define STRIDE 4 //(RGBA)
