#include <stdio.h>
#include <string.h>
#include <vector>

#include "pathtracer_renderer.hh"
#include "pathtracer_scene.hh"
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

int main()
{
    pathtracer::scene_t scene;
    memset(&scene, 0, sizeof(scene));

    scene.width = 1024;
    scene.height = 1024;
    scene.camera_position = vec3_t(2, 0, -15);
    scene.camera_direction = vec3_t(-0.1, 0.2, 1);
    scene.objects = std::vector<pathtracer::object_t*>(0);

    auto r_sphere = create_sphere(vec3_t(5, 3, 50), 1.0f, vec3_t(1.0, 0, 0));
    auto g_sphere = create_sphere(vec3_t(0, 4.5, 100), 10.0f, vec3_t(0, 1.0, 0));
    auto b_sphere = create_sphere(vec3_t(-25, 4.5, 100), 10.0f, vec3_t(0, 0, 1.0));

    pathtracer::object_mesh_t mesh;
    mesh.type = pathtracer::object_type_e::MESH;
    mesh.position = vec3_t(0, 0, 50);
    mesh.color = vec3_t(0, 1.0, 1.0);
    mesh.vtx = new vec3_t[3];
    mesh.vtx[0] = vec3_t(0, -5, 10);
    mesh.vtx[1] = vec3_t(10, 10, 0);
    mesh.vtx[2] = vec3_t(10, -5, 0);
    mesh.vtx_count = 3;

    
    auto floor = create_plane(vec3_t(0, -5, 0), vec3_t(0, 1, 0),
                                    vec3_t(1.0, 1.0, 1.0));

    scene.objects.push_back(&r_sphere);
    scene.objects.push_back(&g_sphere);
    scene.objects.push_back(&b_sphere);
    scene.objects.push_back(&mesh);
    scene.objects.push_back(&floor);

    pathtracer::render_scene(&scene);

    return 0;
}
