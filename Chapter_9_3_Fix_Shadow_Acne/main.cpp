#include <iostream>

#include "rtweekend.h"

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"

int main()
{
    hittable_list world;
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
    world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

    camera cam;
    cam.dAspectRatio = 16.0 / 9.0;
    cam.iImageWidth = 800;
    cam.iSamplesPerPixel=100;
    cam.iMaxDepth=50;

    cam.render(world);

    return 0;
}
