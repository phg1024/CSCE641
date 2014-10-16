#ifndef CAMERAINFO_H
#define CAMERAINFO_H

#include "geometryutils.hpp"
using namespace GeometryUtils;

#include "raytracer.h"

class CameraInfo
{
public:
    CameraInfo();

    Ray generateRay(DblPoint2D& pos);

    DblPoint3D _pos;
    DblVector3D _dir;
    DblVector3D _up;
    double _focalLength;
    double _canvasSize[2];
};

#endif // CAMERAINFO_H
