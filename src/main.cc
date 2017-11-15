#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "defines.hh"
#include "helpers.hh"
#include "renderer.hh"
#include "types.hh"
#include "pathtracer_framework.hh"
#include "vectors.hh"

static pathtracer::object_plane_t
    create_plane(vec3_t pt, vec3_t normal, vec3_t color)
{
    pathtracer::object_plane_t plane;
    plane.type = pathtracer::object_type_e::PLANE;
    plane.position = pt;
    plane.normal = normal;
    plane.color = color;

    return plane;
}

static pathtracer::object_sphere_t
    create_sphere(vec3_t center, float rad, vec3_t color)
{
    pathtracer::object_sphere_t s;
    s.type = pathtracer::object_type_e::SPHERE;
    s.position = center;
    s.radius = rad;
    s.color = color;

    return s;
}

static pathtracer::area_light_t
    create_area_light(vec3_t pos, vec3_t color, double width, double length)
{
    pathtracer::area_light_t l;
    l.type = pathtracer::object_type_e::AREA_LIGHT;
    l.position = pos;
    l.color = color;
    l.size = vec3_t(width, 0.0, length);

    return l;
}

int main()
{
#define WHITE vec3_t(0.95, 0.95, 0.95)
#define RED vec3_t(0.98, 0.2, 0.0)
#define BLUE vec3_t(0.2, 0.65, 0.98)
#define GRAY vec3_t(0.8, 0.8, 0.8)

    std::srand(1);

    pathtracer::scene_t scene;
    memset(&scene, 0, sizeof(scene));

    scene.camera_position = vec3_t(0, 0, -15);
    scene.camera_direction = vec3_t(0, 0, 1);
    scene.objects = std::vector<pathtracer::object_t*>(0);

    auto sphere_front = create_sphere(vec3_t(2, -3.5, 1), 1.5f, WHITE);
    auto sphere_back  = create_sphere(vec3_t(-2.5, -3.5, 8.5), 1.5f, WHITE);

    auto floor = create_plane(vec3_t(0, -5, 0), vec3_t(0, 1, 0), GRAY);
    auto wall_front = create_plane(vec3_t(0, 0, 20), vec3_t(0, 0, -1), WHITE);
    auto wall_back = create_plane(vec3_t(0, 0, -20), vec3_t(0, 0, 1), WHITE);

    auto wall_right = create_plane(vec3_t(5, 0, 0), vec3_t(-1, 0, 0), BLUE);
    auto wall_left = create_plane(vec3_t(-5, 0, 0), vec3_t(1, 0, 0), RED);
    auto roof = create_plane(vec3_t(0, 5, 0), vec3_t(0, -1, 0), GRAY);

    auto light = create_area_light(vec3_t(0.0, 4.9, 1.0), WHITE, 6, 6);

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

    pathtracer::render_scene(&scene, WIDTH, HEIGHT);

    return 0;
}
