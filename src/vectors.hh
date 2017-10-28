#pragma once

#include <stdint.h>

typedef struct vec3 {
    float x, y, z;

    vec3(float x = 0, float y = 0, float z = 0)
        : x(x), y(y), z(z)
    { }

} vec3_t;

vec3_t operator+(vec3_t a, vec3_t b);
vec3_t operator-(vec3_t a);
vec3_t operator-(vec3_t a, vec3_t b);
vec3_t operator*(vec3_t a, float b);
float dot(vec3_t a, vec3_t b);
float magnitude(vec3_t v);
vec3_t normalize(vec3_t v);
