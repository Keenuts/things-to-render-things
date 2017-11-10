#pragma once

#include <stdint.h>
#include <string>
#include <math.h>

typedef struct vec3 {
    union {
        double x;
        double r;
    };
    union {
        double y;
        double g;
    };
    union {
        double z;
        double b;
    };

    vec3(double x = 0, double y = 0, double z = 0)
        : x(x), y(y), z(z)
    { }

} vec3_t;


#define VECTOR_ZERO vec3_t(0, 0, 0)
#define VECTOR_UP vec3_t(0, 1, 0)
#define VECTOR_RIGHT vec3_t(1, 0, 0)
#define VECTOR_FORWARD vec3_t(0, 0, 1)

vec3_t operator+(vec3_t a, vec3_t b);
vec3_t operator-(vec3_t a);
vec3_t operator-(vec3_t a, vec3_t b);
vec3_t operator*(vec3_t a, double b);
vec3_t operator*(double a, vec3_t b);

double dot(vec3_t a, vec3_t b);
double magnitude(vec3_t v);
vec3_t normalize(vec3_t v);
std::string to_string(vec3_t v);
