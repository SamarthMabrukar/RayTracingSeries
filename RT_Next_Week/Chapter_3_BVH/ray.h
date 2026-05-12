#ifndef __HRAY__
#define __HRAY__

#include "vec3.h"

class ray
{
private:
    point3 m_origin;
    vec3 m_direction;
    double m_tm;

public:
    ray() {}
    ray(const point3 &origin, const vec3 &direction, double time) : m_origin(origin), m_direction(direction), m_tm(time) {}
    ray(const point3 &origin, const vec3 &direction) : ray(origin, direction, 0) {}

    const point3 &origin() const { return m_origin; }
    const vec3 &direction() const { return m_direction; }
    double time() const { return m_tm; }

    point3 at(double t) const { return m_origin + t * m_direction; }
};

#endif //__HRAY__