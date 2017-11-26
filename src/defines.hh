#pragma once

//#define USE_RAYTRACER
//#define USE_PATHTRACER
#define USE_BIDIR_PATHTRACER
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
#define IR_RAY_PER_LIGHT 32
#define IR_RAY_DEPTH 1

// Bidirectionnal pathracing 
#define BDPT_MAX_CRAY_DEPTH 3
#define BDPT_MAX_LRAY_DEPTH 2
#define BDPT_SAMPLES 128
#define BDPT_RAY_PER_LIGHT 32
