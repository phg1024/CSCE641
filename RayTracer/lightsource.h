#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H

#include "geometryutils.hpp"
using namespace GeometryUtils;

#include "color.hpp"

class LightSource
{
public:
    enum LightSourceType
    {
        POINT,
        // LINE,
        // BAND,
        // CUBE,
        // SPHERE,
        UNKNOWN
    };

    LightSource();

    static LightSourceType interpretType(const string&);

    DblPoint3D _pos;
    DblColor4 _color;
    LightSourceType _type;
};

#endif // LIGHTSOURCE_H
