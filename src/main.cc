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
    scene.camera_direction = vec3_t(1, 0, 0);
    scene.objects = std::vector<pathtracer::object_t*>(0);

    pathtracer::object_t main_sphere;
    main_sphere.type = pathtracer::object_type_e::SPHERE;
    main_sphere.position = vec3_t(0, 0, 0);
    main_sphere.color = vec3_t(255, 0, 0);

    scene.objects.push_back(&main_sphere);

    pathtracer::render_scene(&scene);

    return 0;
}
