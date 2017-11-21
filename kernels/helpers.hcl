#define GEN_SWAP(Type)        \
static inline                 \
void swap(Type *a, Type *b) { \
  Type c = *a;                \
  *a = *b;                    \
  *b = c;                     \
}

GEN_SWAP(float);

#define FOV 45.0f
#define PI 3.14159265358979323846f
#define DEG2RAD 0.017453292519943295f
#define D_EPSYLON 0.00001
#define F_EPSYLON 0.00001f
#define RAND_MAX 2147483647

static float3 saturate(float3 f)
{
    f.x = clamp(f.x, 0.0f, 1.0f);
    f.y = clamp(f.y, 0.0f, 1.0f);
    f.z = clamp(f.z, 0.0f, 1.0f);
    return f;
}

static float3 mult(float3 m[3], float3 v)
{
    float3 res;
    res.x = m[0].x * v.x + m[0].y * v.y + m[0].z * v.z;
    res.y = m[1].x * v.x + m[1].y * v.y + m[1].z * v.z;
    res.z = m[2].x * v.x + m[2].y * v.y + m[2].z * v.z;
    return res;
}

static float3 rotation(float3 in, float3 angles)
{
    float3 x_rot[3];
    float3 y_rot[3];
    float3 z_rot[3];

    x_rot[0] = (float3)(1.0f,          0.0f,           0.0f);
    x_rot[1] = (float3)(0.0f, cos(angles.x), -sin(angles.x));
    x_rot[2] = (float3)(0.0f, sin(angles.x), cos(angles.x));

    y_rot[0] = (float3)(cos(angles.y),  0.0f,          sin(angles.y));
    y_rot[1] = (float3)(          0.0f,  1.0f,          0.0f);
    y_rot[2] = (float3)(-sin(angles.y), 0.0f,          cos(angles.y));

    z_rot[0] = (float3)(cos(angles.z), -sin(angles.z), 0.0f);
    z_rot[1] = (float3)(sin(angles.z), cos(angles.z),  0.0f);
    z_rot[2] = (float3)(0.0f,          0.0f,           1.0f);

    in = mult(x_rot, mult(z_rot, mult(y_rot, in)));
    return in;
}

static int rand(scene_t *scene)
{
    const int a = 16807;
    const int m = RAND_MAX;

    int seed = scene->last_seed;
    seed = ((long)(seed * a)) % m;
    scene->last_seed = seed;
    return seed;
}

static float rand_0_1(scene_t *scene)
{
    return (float)rand(scene) / (float)RAND_MAX;
}

static float3 get_hemisphere_random(scene_t *scene, float3 dir)
{
    float x, y, z;
    float m;

    do {
        x = 2.0f * rand_0_1(scene) - 1.0f;
        y = 2.0f * rand_0_1(scene) - 1.0f;
        z = 2.0f * rand_0_1(scene) - 1.0f;
        m = length((float3)(x, y, z));
    } while (m > 1.0f);

    float3 d = normalize((float3)(x, y, z));
    if (dot(d, dir) < 0.0f)
        d = -d;

    return d;
}
