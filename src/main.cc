#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "defines.hh"
#include "framework.hh"
#include "helpers.hh"
#include "renderer.hh"
#include "types.hh"
#include "vectors.hh"
#include "viewer.hh"

static RE::object_plane_t create_infinite_plane(vec3_t pt, vec3_t normal,
                                                RE::material_t mlt)
{
    RE::object_plane_t plane;
    plane.type = RE::object_type_e::PLANE;
    plane.position = pt;
    plane.normal = normal;
    plane.mlt = mlt;

    return plane;
}

static RE::object_sphere_t create_sphere(vec3_t center, float rad, RE::material_t mlt)
{
    RE::object_sphere_t s;
    s.type = RE::object_type_e::SPHERE;
    s.position = center;
    s.radius = rad;
    s.mlt = mlt;

    return s;
}

static RE::area_light_t create_area_light(vec3_t pos, RE::material_t mlt,
                                      float power, float width, float length)
{
    RE::area_light_t l;
    l.type = RE::object_type_e::AREA_LIGHT;
    l.position = pos;
    l.mlt = mlt;
    l.size = vec3_t(width, 0.0f, length);
    l.power = power;

    return l;
}

static RE::object_mesh_t create_mesh(vec3_t position, vec3_t rotation, RE::material_t mlt,
                                     vec3_t *vtx, vec3_t *uv,
                                     uint64_t vtx_count)
{
    RE::object_mesh_t m;
    m.type = RE::object_type_e::MESH;
    m.position = position;
    m.rotation = rotation;
    m.mlt = mlt;
    m.vtx = vtx;
    m.uv = uv;
    m.vtx_count = vtx_count;

    return m;
}

static RE::object_mesh_t create_plane(vec3_t position, vec3_t size, vec3_t rotation,
                                  RE::material_t mlt)
{
    vec3_t *vtx = new vec3_t[6];
    vec3_t *uv = new vec3_t[6];

    vtx[0] = vec3_t(-0.5f * size.x, -0.5f * size.y);
    vtx[1] = vec3_t(-0.5f * size.x,  0.5f * size.y);
    vtx[2] = vec3_t( 0.5f * size.x, -0.5f * size.y); 
    vtx[3] = vec3_t( 0.5f * size.x, -0.5f * size.y);
    vtx[4] = vec3_t(-0.5f * size.x,  0.5f * size.y);
    vtx[5] = vec3_t( 0.5f * size.x,  0.5f * size.y); 

    uv[0] = vec3_t(0, 0);
    uv[1] = vec3_t(0, 1);
    uv[2] = vec3_t(1, 0);
    uv[3] = vec3_t(1, 0);
    uv[4] = vec3_t(0, 1);
    uv[5] = vec3_t(1, 1);

    return create_mesh(position, rotation, mlt, vtx, uv, 6);
}

int main()
{
#define RED vec3_t(0.98f,   0.2f, 0.0f)
#define BLUE vec3_t(0.2f,   0.65f, 0.98f)
#define GRAY vec3_t(0.8f,   0.8f, 0.8f)

    RE::material_t gray, white, red, blue, light_white;

    white.diffuse = WHITE;
    white.has_texture = false;

    gray.diffuse = GRAY;
    gray.has_texture = false;

    red.diffuse = RED;
    red.has_texture = false;

    blue.diffuse = BLUE;
    blue.has_texture = false;

    light_white.emission = WHITE;
    light_white.has_texture = false;

    std::srand(1);

    RE::scene_t scene;
    memset(&scene, 0, sizeof(scene));

    scene.camera_position = vec3_t(0, 0, -15);
    scene.camera_direction = vec3_t(0, 0, 1);
    scene.objects = std::vector<RE::object_t*>(0);

    auto sphere_front = create_sphere(vec3_t(2, -3.5, -1), 1.5f, white);
    auto sphere_back  = create_sphere(vec3_t(-2, -3.0, 3.5), 2.0f, white);

    (void)create_infinite_plane;

    auto floor = create_plane(vec3_t(0, -5, 0), vec3_t(10.2, 10.2), vec3_t(90, 0, 0), gray);
    auto roof  = create_plane(vec3_t(0,  5, 0), vec3_t(10.2, 10.2), vec3_t(-90, 0, 0), gray);
    auto front = create_plane(vec3_t(0, 0, 5),  vec3_t(10, 10), vec3_t(0, 0, 0), white);

    auto right = create_plane(vec3_t(5, 0, 0), vec3_t(10, 10), vec3_t(0, 90, 0), blue);
    auto left = create_plane(vec3_t(-5, 0, 0), vec3_t(10, 10), vec3_t(0, -90, 0), red);

    auto light = create_area_light(vec3_t(0, 4.5f, -1), light_white, 5.0, 5, 5);

    scene.objects.push_back(&sphere_front);
    scene.objects.push_back(&sphere_back);
    scene.objects.push_back(&floor);
    scene.objects.push_back(&roof);
    scene.objects.push_back(&front);
    scene.objects.push_back(&right);
    scene.objects.push_back(&left);

    scene.objects.push_back(&light);


#if defined(RENDER_PARTIAL)
    struct RE::area render_area = {
        (uint32_t)(WIDTH * 0.25),
        (uint32_t)(HEIGHT * 0.25),
        (uint32_t)(WIDTH * 0.5),
        (uint32_t)(HEIGHT * 0.5)
    };
    RE::render_scene(&scene, WIDTH, HEIGHT, &render_area);
#else
    RE::render_scene(&scene, WIDTH, HEIGHT, nullptr);
#endif


    return 0;
}
