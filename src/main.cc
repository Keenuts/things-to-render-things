#include <stdio.h>
#include <string.h>
#include <vector>

#include "pathtracer_renderer.hh"
#include "pathtracer_scene.hh"
#include "vectors.hh"

int main()
{
    pathtracer::scene_t scene;
    memset(&scene, 0, sizeof(scene));

    scene.width = 512;
    scene.height = 512;
    scene.camera_position = vec3_t(0, 0, 0);
    scene.camera_direction = vec3_t(0, 0, 1);
    scene.objects = std::vector<pathtracer::object_t*>(0);

    pathtracer::object_sphere_t first_sphere;
    first_sphere.type = pathtracer::object_type_e::SPHERE;
    first_sphere.position = vec3_t(-25, 0, 100);
    first_sphere.color = vec3_t(0, 0, 255); //B
    first_sphere.radius = 10.0f;

    pathtracer::object_sphere_t second_sphere;
    second_sphere.type = pathtracer::object_type_e::SPHERE;
    second_sphere.position = vec3_t(10, 0, 55);
    second_sphere.color = vec3_t(0, 255, 0); //G
    second_sphere.radius = 5.0f;

    pathtracer::object_sphere_t third_sphere;
    third_sphere.type = pathtracer::object_type_e::SPHERE;
    third_sphere.position = vec3_t(0, 0, 30);
    third_sphere.color = vec3_t(255, 0, 0); //R
    third_sphere.radius = 1.0f;

    scene.objects.push_back(&first_sphere);
    scene.objects.push_back(&second_sphere);
    scene.objects.push_back(&third_sphere);

    pathtracer::render_scene(&scene);

    return 0;
}
