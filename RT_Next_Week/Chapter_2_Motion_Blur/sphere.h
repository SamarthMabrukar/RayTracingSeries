#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "rtweekend.h"

class sphere : public hittable
{
private:
    ray center;
    double radius;
    shared_ptr<material> mat;

public:
    // Stationary Sphere
    sphere(const point3& static_center, double radius, shared_ptr<material> mat)
      : center(static_center, vec3(0,0,0)), radius(std::fmax(0,radius)), mat(mat) {}

    // Moving Sphere
    sphere(const point3& center1, const point3& center2, double radius,
           shared_ptr<material> mat)
      : center(center1, center2 - center1), radius(std::fmax(0,radius)), mat(mat) {}

    bool hit(const ray &vRay, interval ray_t, hit_record &rec) const override
    {
        point3 current_center = center.at(vRay.time());
        vec3 oc = current_center - vRay.origin();
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
        rec.p = vRay.at(rec.t);
        vec3 outward_normal = (rec.p - current_center) / radius;
        rec.set_face_normal(vRay, outward_normal);
        rec.mat = mat;

        return true;
    }
};

#endif