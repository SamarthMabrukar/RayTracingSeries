#include <iostream>

#include "vec3.h"
#include "color.h"
#include "ray.h"

double hit_sphere(const point3 &vCenter, double dRadius, const ray &vRay)
{
    vec3 oc = vCenter - vRay.origin();
    // auto a = dot(vRay.direction(), vRay.direction());
    auto a = vRay.direction().squared_length();
    // auto b = -2.0 * dot(vRay.direction(), oc);
    auto h = dot(vRay.direction(), oc);
    auto c = oc.squared_length() - dRadius * dRadius;
    auto discriminant = h * h - a * c;
    // auto discriminant = b * b - 4 * a * c;

    if (discriminant < 0)
    {
        return -1.0;
    }
    else
    {
        // return ((-b) - std::sqrt(discriminant)) / (2.0 * a);
        return ((h)-std::sqrt(discriminant)) / (a);
    }
}

color ray_color(const ray &r)
{
    // Ray hit color
    double t = hit_sphere(point3(0, 0, -1), 0.5, r);
    if (t > 0.0)
    {
        vec3 N = unit_vector(r.at(t) - vec3(0, 0, -1));
        return 0.5 * color(N.x() + 1, N.y() + 1, N.z() + 1);
    }

    // Background color
    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0); // conversion if [-1,1] to [0,1]
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}

int main()
{

    double dAspectRatio = 16.0 / 9.0;
    int iImageWidth = 800;

    // Calculate Image Height
    int iImageHeight = int(iImageWidth / dAspectRatio);
    iImageHeight = (iImageHeight < 1) ? 1 : iImageHeight;

    // viewport widthd less than one are okay since they are real valued.
    double dFocalLength = 1.0;
    double dViewportHeight = 2.0;
    double dViewportWidth = dViewportHeight * (double(iImageWidth) / iImageHeight);
    point3 vCameraCenter = point3(0.0, 0.0, 0.0);

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    vec3 vViewport_u = vec3(dViewportWidth, 0.0, 0.0);
    vec3 vViewport_v = vec3(0.0, -dViewportHeight, 0.0); // Y axis is inverted

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    vec3 vPixelDelta_u = vViewport_u / iImageWidth;
    vec3 vPixelDelta_v = vViewport_v / iImageHeight;

    // Calculate the location of the upper left pixel.
    vec3 vViewport_upper_left = vCameraCenter - vec3(0.0, 0.0, dFocalLength) - (vViewport_u / 2) - (vViewport_v / 2);
    vec3 vPixel00_loc = vViewport_upper_left + 0.5 * (vPixelDelta_u + vPixelDelta_v);

    std::cout
        << "P3\n"
        << iImageWidth << " " << iImageHeight << "\n255\n";

    for (int j = 0; j < iImageHeight; j++)
    {
        std::clog << "\rScanlines remaining: " << (iImageHeight - j) << ' ' << std::flush;
        for (int i = 0; i < iImageWidth; i++)
        {
            auto pixel_center = vPixel00_loc + (i * vPixelDelta_u) + (j * vPixelDelta_v);
            auto ray_direction = pixel_center - vCameraCenter;
            ray r(vCameraCenter, ray_direction);

            color pixel_color = ray_color(r);
            write_color(std::cout, pixel_color);
        }
    }

    std::clog << "\rDone.                 \n";

    return 0;
}
