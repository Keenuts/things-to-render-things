#pragma once

#include <math.h>
#include <stdint.h>
#include <string>

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

    double& operator[](int i);

    void operator*=(double a);
    void operator*=(struct vec3 v);
    void operator+=(struct vec3 v);
    void operator-=(struct vec3 v);
} vec3_t;


#define VECTOR_ZERO vec3_t(0, 0, 0)
#define VECTOR_UP vec3_t(0, 1, 0)
#define VECTOR_RIGHT vec3_t(1, 0, 0)
#define VECTOR_FORWARD vec3_t(0, 0, 1)
#define BLACK vec3_t(0.0, 0.0, 0.0)
#define WHITE vec3_t(1.0, 1.0, 1.0)

vec3_t operator+(vec3_t a, vec3_t b);
vec3_t operator-(vec3_t a);
vec3_t operator-(vec3_t a, vec3_t b);
vec3_t operator*(vec3_t a, double b);
vec3_t operator*(double a, vec3_t b);
vec3_t operator/(double a, vec3_t b);
vec3_t operator/(vec3_t a, double b);
vec3_t operator*(vec3_t a, vec3_t b);

double dot(vec3_t a, vec3_t b);
vec3_t cross(vec3_t a, vec3_t b);
double magnitude(vec3_t v);
vec3_t normalize(vec3_t v);
std::string to_string(vec3_t v);

vec3_t reflect(vec3_t i, vec3_t n);
vec3_t saturate(vec3_t c);

vec3_t rotate(vec3_t in, vec3_t angles);
vec3_t get_sphere_random(void);
vec3_t get_hemisphere_random(vec3_t dir);
