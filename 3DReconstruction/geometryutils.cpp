#include "geometryutils.hpp"

namespace GeometryUtils{

void assignPolygonOrientation(DblPolygon &polygon)
{
    // at least 3 points to form a polygon
    assert(polygon.points.size() >= 3);

    size_t offset = floor(polygon.points.size() / 3.0);

    list<DblPoint2D>::iterator it = polygon.points.begin();
    DblPoint2D p0 = (*it);
    for(size_t i=0;i<offset;i++) it++;
    DblPoint2D p1 = (*it);
    for(size_t i=0;i<offset;i++) it++;
    DblPoint2D p2 = (*it);

    DblVector2D p0p1, p0p2;
    p0p1 = p1 - p0;
    p0p2 = p2 - p0;

    double cp = crossProduct(p0p1, p0p2);

    if(cp >= 0)
        polygon.isClockWise = true;
    else
        polygon.isClockWise = false;
}

double dotProduct(const DblVector3D& v1, const DblVector3D& v2)
{
    return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
}

double dotProduct(const DblVector2D& v1, const DblVector2D& v2)
{
    return v1.x() * v2.x() + v1.y() * v2.y();
}

DblVector3D crossProduct(const DblVector3D& v1, const DblVector3D& v2)
{
    return DblVector3D(v1.y() * v2.z() - v1.z() * v2.y(),
                       v1.z() * v2.x() - v1.x() * v2.z(),
                       v1.x() * v2.y() - v1.y() * v2.x());
}

double crossProduct(const DblVector2D& v1, const DblVector2D& v2)
{
    return v1.x() * v2.y() - v2.x() * v1.y();
}

}
