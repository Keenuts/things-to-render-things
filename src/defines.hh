#pragma once

#define USE_RAYTRACER
//#define USE_PATHTRACER
//#define USE_MDT

// Enable this to only render a part of the front sphere
//#define RENDER_PARTIAL

#define WIDTH 256
#define HEIGHT 256
#define STRIDE 4 //(RGBA)
#define MAX_THREADS 4

// Pathtracer setings
#define PT_SAMPLES 128
#define PT_MAX_DEPTH 3

// Raytracer settings
//#define RT_ENABLE_SHADOWS
#define RT_SOFT_SHADOW_SAMPLES 1
#define RT_SOFT_SHADOW_RADIUS 0.01

// MDT settings
#define IR_RAY_PER_LIGHT 16
#define IR_RAY_DEPTH 2
