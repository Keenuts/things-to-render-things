#include <stdio.h>
#include <string.h>
#include <vector>

#include "pathtracer_renderer.hh"
#include "pathtracer_scene.hh"
#include "vectors.hh"

static pathtracer::object_plane_t
    create_plane(vec3_t pt, vec3_t n, vec3_t color)
{
    pathtracer::object_plane_t plane;
    plane.type = pathtracer::object_type_e::PLANE;
    plane.position = pt;
    plane.color = color;

    return plane;
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

    pathtracer::object_sphere_t first_sphere;
    first_sphere.type = pathtracer::object_type_e::SPHERE;
    first_sphere.position = vec3_t(-25, 4.5, 100);
    first_sphere.color = vec3_t(0, 0, 1.0); //B
    first_sphere.radius = 10.0f;

    pathtracer::object_sphere_t second_sphere;
    second_sphere.type = pathtracer::object_type_e::SPHERE;
    second_sphere.position = vec3_t(0, 4.5, 100);
    second_sphere.color = vec3_t(0, 1.0, 0); //G
    second_sphere.radius = 10.0f;

    pathtracer::object_sphere_t third_sphere;
    third_sphere.type = pathtracer::object_type_e::SPHERE;
    third_sphere.position = vec3_t(5, 3, 50);
    third_sphere.color = vec3_t(1.0, 0, 0); //R
    third_sphere.radius = 1.0f;

    pathtracer::object_mesh_t first_mesh;
    first_mesh.type = pathtracer::object_type_e::MESH;
    first_mesh.position = vec3_t(0, -5, 20);
    first_mesh.color = vec3_t(0, 1.0, 1.0);
    first_mesh.vtx = new vec3_t[3];
    first_mesh.vtx_count = 3;

    
    auto first_plane = create_plane(vec3_t(0, -1, 0), vec3_t(0, 1, 0),
                                    vec3_t(1.0, 1.0, 1.0));

    pathtracer::object_plane_t second_plane;
    second_plane.type = pathtracer::object_type_e::PLANE;
    second_plane.position = vec3_t(-20, 0, 10);
    second_plane.color = vec3_t(1.0, 0.0, 0.0);
    second_plane.a = vec3_t(0, 0, 0);
    second_plane.b = vec3_t(0, 1, 1);
    second_plane.c = vec3_t(0, 0, 1);


    scene.objects.push_back(&first_sphere);
    scene.objects.push_back(&second_sphere);
    scene.objects.push_back(&third_sphere);
    scene.objects.push_back(&first_plane);
    scene.objects.push_back(&second_plane);

    pathtracer::render_scene(&scene);

    return 0;
}
