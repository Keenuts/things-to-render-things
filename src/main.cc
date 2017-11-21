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

static RE::object_plane_t create_plane(vec3_t pt, vec3_t normal, RE::material_t mlt)
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

int main()
{
#define RED vec3_t(0.98f,   0.2f, 0.0f)
#define BLUE vec3_t(0.2f,   0.65f, 0.98f)
#define GRAY vec3_t(0.8f,   0.8f, 0.8f)

    RE::material_t gray, white, red, blue, light_white;

    white.diffuse = WHITE;
    gray.diffuse = GRAY;
    red.diffuse = RED;
    blue.diffuse = BLUE;
    light_white.emission = WHITE;

    std::srand(1);

    RE::scene_t scene;
    memset(&scene, 0, sizeof(scene));

    scene.camera_position = vec3_t(0, 0, -15);
    scene.camera_direction = vec3_t(0, 0, 1);
    scene.objects = std::vector<RE::object_t*>(0);

    auto sphere_front = create_sphere(vec3_t(2, -3.5, 1), 1.5f, white);
    auto sphere_back  = create_sphere(vec3_t(-2.5, -3.5, 8.5), 1.5f, white);

    auto floor = create_plane(vec3_t(0, -5, 0), vec3_t(0, 1, 0), gray);
    auto wall_front = create_plane(vec3_t(0, 0, 20), vec3_t(0, 0, -1), white);
    auto wall_back = create_plane(vec3_t(0, 0, -20), vec3_t(0, 0, 1), white);

    auto wall_right = create_plane(vec3_t(5, 0, 0), vec3_t(-1, 0, 0), blue);
    auto wall_left = create_plane(vec3_t(-5, 0, 0), vec3_t(1, 0, 0), red);
    auto roof = create_plane(vec3_t(0, 5, 0), vec3_t(0, -1, 0), gray);

    auto light = create_area_light(vec3_t(0.0, 4.9, 1.0), light_white, 3.0, 6, 6);

    scene.objects.push_back(&sphere_front);
    scene.objects.push_back(&sphere_back);
    scene.objects.push_back(&floor);
    scene.objects.push_back(&roof);
    scene.objects.push_back(&wall_right);
    scene.objects.push_back(&wall_left);
    scene.objects.push_back(&wall_front);
    scene.objects.push_back(&wall_back);
    scene.objects.push_back(&light);

    (void)sphere_front;
    (void)sphere_back;
    (void)floor;
    (void)roof;
    (void)wall_right;
    (void)wall_left;
    (void)wall_front;
    (void)wall_back;
    (void)light;

#if 0
    struct RE::area render_area = { 350, 350, 16, 128 };
    RE::render_scene(&scene, WIDTH, HEIGHT, &render_area);
#else
    RE::render_scene(&scene, WIDTH, HEIGHT, nullptr);
#endif


    return 0;
}
