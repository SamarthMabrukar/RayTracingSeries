#include <iostream>

#include "rtweekend.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"

color ray_color(const ray &r, const hittable &world)
{
    // Ray hit color
    hit_record rec;
    if (world.hit(r, interval(0, infinity), rec))
    {
        return 0.5 * (rec.normal + color(1, 1, 1));
    }

    // Background color
    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0); // conversion if [-1,1] to [0,1]
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}

int main()
{

    // World
    hittable_list world;
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
    world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

    camera cam;
    cam.dAspectRatio = 16.0 / 9.0;
    cam.iImageWidth = 800;
    cam.iSamplesPerPixel = 100;
    cam.iMaxDepth = 50;

    cam.render(world);

    return 0;
}
