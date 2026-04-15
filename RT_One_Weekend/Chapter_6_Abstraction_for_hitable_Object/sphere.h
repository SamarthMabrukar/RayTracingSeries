#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "rtweekend.h"

class sphere : public hittable
{
private:
    point3 center;
    double radius;

public:
    sphere(const point3 &center, double radius) : center(center), radius(std::fmax(0, radius)) {}

    bool hit(const ray &vRay, interval ray_t, hit_record &rec) const override
    {
        vec3 oc = center - vRay.origin();
        auto a = vRay.direction().squared_length();
        auto h = dot(vRay.direction(), oc);
        auto c = oc.squared_length() - radius * radius;
        auto discriminant = h * h - a * c;

        if (discriminant < 0)
        {
            return false;
        }

        auto sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root))
        {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root))
            {
                return false;
            }
        }

        rec.t = root;
        rec.point = vRay.at(rec.t);
        rec.normal = (rec.point - center) / radius;
        vec3 outward_normal = (rec.point - center) / radius;
        rec.set_face_normal(vRay, outward_normal);

        return true;
    }
};

#endif