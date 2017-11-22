#pragma once

#include "vectors.hh"

typedef struct mat3 {
    vec3_t lines[3];

    mat3();
    mat3(float v);
    mat3(vec3_t values[3]);

    vec3_t& operator[](int i);
} mat3_t;

vec3_t operator*(mat3_t, vec3_t);
