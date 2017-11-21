#pragma once

#define USE_RAYTRACER
//#define USE_PATHTRACER
//#define USE_MDT

#define WIDTH 512
#define HEIGHT 512
#define STRIDE 4 //(RGBA)
#define MAX_THREADS 4

// Pathtracer setings
#define PT_SAMPLES 1024
#define PT_MAX_DEPTH 4

// Raytracer settings
#define SOFT_SHADOW_SAMPLES 1
#define SOFT_SHADOW_RADIUS 0.01

// MDT settings
#define IR_RAY_PER_LIGHT 16
