#include <assert.h>

#include "vectors.hh"
#include "matrix.hh"

mat3_t::mat3(vec3_t values[3])
{
    for (uint32_t i = 0; i < 3; i++)
        lines[i] = values[i];
}

mat3_t::mat3(double v)
{
    for (uint32_t i = 0; i < 3; i++)
        lines[i] = vec3_t(v, v, v);
}

mat3_t::mat3() : mat3(0.0)
{ }

vec3_t& mat3_t::operator[](int i) {
    assert(i >= 0 && i < 3 && "Invalid subscript range");
    return lines[i];
}

vec3_t operator*(mat3_t m, vec3_t v)
{
    vec3_t res;
    for (uint32_t i = 0; i < 3; i++)
        res[i] = m[i][0] * v[0] + m[i][1] * v[1] + m[i][2] * v[2];
    return res;
}
