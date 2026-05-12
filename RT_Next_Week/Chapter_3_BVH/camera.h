#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"

class camera
{
public:
    double dAspectRatio = 1.0; // Ratio of image width over height
    int iImageWidth = 100;     // Rendered image width in pixel count
    int iSamplesPerPixel = 10; // Count of random samples for each pixel
    int iMaxDepth = 10;        // Maximum number of ray bounces into scene

    double vfov = 90;                  // Vertical view angle (field of view)
    point3 lookfrom = point3(0, 0, 0); // Point camera is looking from
    point3 lookat = point3(0, 0, -1);  // Point camera is looking at
    vec3 vup = vec3(0, 1, 0);          // Camera-relative "up" direction

    double defocus_angle = 0; // Variation angle of rays through each pixel
    double focus_dist = 10;   // Distance from camera lookfrom point to plane of perfect focus

    void render(const hittable &world)
    {
        initialize();

        std::cout << "P3\n"
                  << iImageWidth << ' ' << iImageHeight << "\n255\n";

        for (int j = 0; j < iImageHeight; j++)
        {
            std::clog << "\r  Scanlines remaining: " << (iImageHeight - j) << ' ' << std::flush;
            for (int i = 0; i < iImageWidth; i++)
            {
                color pixel_color(0, 0, 0);
                for (int sample = 0; sample < iSamplesPerPixel; sample++)
                {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, iMaxDepth, world);
                }
                write_color(std::cout, dPixelSamplesScale * pixel_color);
            }
        }

        std::clog << "\r\n Done. \n";
    }

private:
    int iImageHeight{};        // Rendered image height
    double dPixelSamplesScale; // Color scale factor for a sum of pixel samples
    point3 vCameraCenter;      // Camera center
    vec3 vPixel00_loc;         // Location of pixel 0, 0
    vec3 vPixelDelta_u;        // Offset to pixel to the right
    vec3 vPixelDelta_v;        // Offset to pixel below
    vec3 u, v, w;              // Camera frame basis vectors

    vec3 defocus_disk_u; // Defocus disk horizontal radius
    vec3 defocus_disk_v; // Defocus disk vertical radius

    void initialize()
    {
        iImageHeight = int(iImageWidth / dAspectRatio);
        iImageHeight = (iImageHeight < 1) ? 1 : iImageHeight;

        dPixelSamplesScale = 1.0 / iSamplesPerPixel;

        vCameraCenter = lookfrom;

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Determine viewport dimensions from vertical field of view (matches the book).
        // auto dFocalLength = 1.0;
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / 2);
        auto dViewportHeight = 2 * h * focus_dist;
        auto dViewportWidth = dViewportHeight * (double(iImageWidth) / iImageHeight);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        auto vViewport_u = dViewportWidth * u;   // Vector across viewport horizontal edge
        auto vViewport_v = dViewportHeight * -v; // Vector down viewport vertical edge

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        vPixelDelta_u = vViewport_u / iImageWidth;
        vPixelDelta_v = vViewport_v / iImageHeight;

        // Calculate the location of the upper left pixel.
        auto vViewport_upper_left =
            vCameraCenter - (focus_dist * w) - vViewport_u / 2 - vViewport_v / 2;
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

        auto ray_origin = (defocus_angle <= 0) ? vCameraCenter : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;
        auto ray_time = random_double();

        return ray(ray_origin, ray_direction, ray_time);
    }

    vec3 sample_square() const
    {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const
    {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return vCameraCenter + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    color ray_color(const ray &r, int depth, const hittable &world) const
    {
        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return color(0, 0, 0);

        hit_record rec;

        if (world.hit(r, interval(0.001, infinity), rec))
        {
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth - 1, world);
            return color(0, 0, 0);
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }
};

#endif
