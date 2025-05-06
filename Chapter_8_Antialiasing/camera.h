#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "ray.h"

class camera
{
public:
    // double dAspectRatio = 16.0 / 9.0;
    double dAspectRatio = 1.0;
    int iImageWidth = 800;
    int iSamplesPerPixel = 10; // Count of random samples for each pixel
    int iMaxDepth = 10;        // Maximum number of ray bounces into scene

    double vfov = 90;                  // Vertical view angle (field of view)
    point3 lookFrom = point3(0, 0, 0); // Point camera is looking from
    point3 lookat = point3(0, 0, -1);  // Point camera is looking at
    vec3 vup = vec3(0, 1, 0);          // Camera-relative "up" direction

    double dDefocusAngle = 0; // Variation angle of rays through each pixel
    double dFocusDist = 10;   // Distance from camera lookfrom point to plane of perfect focus

    void render(const hittable &world)
    {
        initialize();

        std::cout
            << "P3\n"
            << iImageWidth << " " << iImageHeight << "\n255\n";

        for (int j = 0; j < iImageHeight; j++)
        {
            std::clog << "\rScanlines remaining: " << (iImageHeight - j) << ' ' << std::flush;
            for (int i = 0; i < iImageWidth; i++)
            {
                /*auto pixel_center = vPixel00_loc + (i * vPixelDelta_u) + (j * vPixelDelta_v);
                auto ray_direction = pixel_center - vCameraCenter;
                ray r(vCameraCenter, ray_direction);

                color pixel_color = ray_color(r, world);
                write_color(std::cout, pixel_color);*/
                color pixel_color(0, 0, 0);
                for (int iSample = 0; iSample < iSamplesPerPixel; iSample++)
                {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, world);
                }
                write_color(std::cout, dPixelSampleScale * pixel_color);
            }
        }

        std::clog << "\rDone.                 \n";
    }

private:
    int iImageHeight;
    double dPixelSampleScale; // Color scale factor for a sum of pixel samples
    point3 vCameraCenter;
    vec3 vPixel00_loc;
    vec3 vPixelDelta_u;
    vec3 vPixelDelta_v;
    vec3 u, v, w;        // Camera frame basis vectors
    vec3 defocus_disk_u; // Defocus disk horizontal radius
    vec3 defocus_disk_v; // Defocus disk vertical radius

    void initialize()
    {
        // Calculate Image Height
        iImageHeight = int(iImageWidth / dAspectRatio);
        iImageHeight = (iImageHeight < 1) ? 1 : iImageHeight;

        dPixelSampleScale = 1.0 / iSamplesPerPixel;

        vCameraCenter = lookFrom;

        // Camera
        // viewport widthd less than one are okay since they are real valued.
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / 2);
        double dFocalLength = 1.0;
        double dViewportHeight = 2.0 * h * dFocusDist;
        double dViewportWidth = dViewportHeight * (double(iImageWidth) / iImageHeight);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 vViewport_u = dViewportWidth * u;
        vec3 vViewport_v = dViewportHeight * -v; // Y axis is inverted

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        vec3 vPixelDelta_u = vViewport_u / iImageWidth;
        vec3 vPixelDelta_v = vViewport_v / iImageHeight;

        // Calculate the location of the upper left pixel.
        vec3 vViewport_upper_left = vCameraCenter - (dFocusDist * w) - (vViewport_u / 2) - (vViewport_v / 2);
        vPixel00_loc = vViewport_upper_left + 0.5 * (vPixelDelta_u + vPixelDelta_v);

        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    ray get_ray(int i, int j) const
    {
        // Construct a camera ray originating from the defocus disk and directed at a randomly
        // sampled point around the pixel location i, j.

        auto offset = sample_square();
        auto pixel_sample = vPixel00_loc + ((i + offset.x()) * vPixelDelta_u) + ((j + offset.y()) * vPixelDelta_v);

        // auto ray_origin = (defocus_angle <= 0) ? vCameraCenter : defocus_disk_sample();
        auto ray_origin = vCameraCenter;
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    vec3 sample_square() const
    {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    /*vec3 sample_disk(double radius) const
    {
        // Returns a random point in the unit (radius 0.5) disk centered at the origin.
        return radius * random_in_unit_disk();
    }

    point3 defocus_disk_sample() const
    {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }*/

    color ray_color(const ray &r, const hittable &world) const
    {

        hit_record rec;

        if (world.hit(r, interval(0.001, infinity), rec))
        {
            return 0.5 * (rec.normal + color(1, 1, 1));
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }
};

#endif