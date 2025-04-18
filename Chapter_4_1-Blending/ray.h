#ifndef __HRAY__
#define __HRAY__

#include "vec3.h"

class ray
{
private:
    point3 m_origin;
    vec3 m_direction;

public:
    ray() {}
    ray(const point3 &origin, const vec3 &direction) : m_origin(origin), m_direction(direction) {}

    const point3 &origin() const { return m_origin; }
    const vec3 &direction() const { return m_direction; }

    point3 at(double t) const { return m_origin + t * m_direction; }
};

#endif //__HRAY__