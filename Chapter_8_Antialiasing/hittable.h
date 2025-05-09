#ifndef HITTABLE_H
#define HITTABLE_H

#include "rtweekend.h"

class hit_record
{
public:
    point3 point;
    vec3 normal;
    double t;
    bool front_face;

    void set_face_normal(const ray &vRay, const vec3 &outward_normal)
    {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.

        front_face = dot(vRay.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable
{
public:
    virtual ~hittable() = default;
    virtual bool hit(const ray &vRay, interval ray_t, hit_record &rec) const = 0;
};

#endif