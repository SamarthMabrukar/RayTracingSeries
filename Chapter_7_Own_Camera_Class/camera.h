#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"

class camera
{
public:
    double dAspectRatio = 1.0; // Ratio of image width over height
    int iImageWidth = 100;     // Rendered image width in pixel count

    void render(const hittable &world)
    {
        initialize();

        std::cout << "P3\n" << iImageWidth << ' ' << iImageHeight << "\n255\n";

        for (int j = 0; j < iImageHeight; j++)
        {
            std::clog << "\rScanlines remaining: " << (iImageHeight - j) << ' ' << std::flush;
            for (int i = 0; i < iImageWidth; i++)
            {
                auto pixel_center = vPixel00_loc + (i * vPixelDelta_u) + (j * vPixelDelta_v);
                auto ray_direction = pixel_center - vCameraCenter;
                ray r(vCameraCenter, ray_direction);

                color pixel_color = ray_color(r, world);
                write_color(std::cout, pixel_color);
            }
        }

        std::clog << "\rDone. \n";
    }

private:
    int iImageHeight{};   // Rendered image height
    point3 vCameraCenter; // Camera center
    vec3 vPixel00_loc;    // Location of pixel 0, 0
    vec3 vPixelDelta_u;   // Offset to pixel to the right
    vec3 vPixelDelta_v;   // Offset to pixel below

    void initialize()
    {
        iImageHeight = int(iImageWidth / dAspectRatio);
        iImageHeight = (iImageHeight < 1) ? 1 : iImageHeight;

        vCameraCenter = point3(0, 0, 0);

        // Determine viewport dimensions.
        auto dFocalLength = 1.0;
        auto dViewportHeight = 2.0;
        auto dViewportWidth = dViewportHeight * (double(iImageWidth) / iImageHeight);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        auto vViewport_u = vec3(dViewportWidth, 0, 0);
        auto vViewport_v = vec3(0, -dViewportHeight, 0);

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        vPixelDelta_u = vViewport_u / iImageWidth;
        vPixelDelta_v = vViewport_v / iImageHeight;

        // Calculate the location of the upper left pixel.
        auto vViewport_upper_left =
            vCameraCenter - vec3(0, 0, dFocalLength) - vViewport_u / 2 - vViewport_v / 2;
        vPixel00_loc = vViewport_upper_left + 0.5 * (vPixelDelta_u + vPixelDelta_v);
    }

    color ray_color(const ray &r, const hittable &world) const
    {
        hit_record rec;

        if (world.hit(r, interval(0, infinity), rec))
        {
            return 0.5 * (rec.normal + color(1, 1, 1));
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }
};

#endif
