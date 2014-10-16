#include "camerainfo.h"

#include "matrixutil.hpp"
using namespace MatrixUtils;

CameraInfo::CameraInfo()
{
}

Ray CameraInfo::generateRay(GeometryUtils::DblPoint2D &point)
{
    double phi, theta;
    phi = acos( dotProduct( normalize( _dir ), DblVector3D(0, -1, 0) ) );
    phi -= PI / 2.0;
    theta = atan2(_dir.x(), _dir.z());
    theta -= PI;

    DblVector4D transformedUp = DblVector4D(_up, 1);
    transformedUp = makeXRotationMatrix(phi) * transformedUp;
    transformedUp = makeYRotationMatrix(theta) * transformedUp;

//    cout << "is orthogonal: " << dotProduct( _dir, transformedUp ) << endl;
//    DblVector4D transformedDir = DblVector4D(0, 0, -1, 1);
//    transformedDir = makeXRotationMatrix(phi) * transformedDir;
//    transformedDir = makeYRotationMatrix(theta) * transformedDir;
//    cout << "is equal: " << length( normalize( transformedDir.xyz() ) - normalize(_dir) ) << endl;

    DblVector3D _horizontal = normalize( crossProduct(_dir, transformedUp.xyz()) );

    DblPoint3D _canvasCenter = _pos + normalize( _dir ) *  _focalLength;

    DblPoint3D origin = _canvasCenter
            + _horizontal * (point.x() - 0.5) * _canvasSize[0]
            + transformedUp.xyz() * (0.5 - point.y()) * _canvasSize[1];

    DblVector3D _direction = normalize(origin - _pos);

    Ray r(_direction, origin);
    r.refractionRate() = 1.0;

    return r;
}
