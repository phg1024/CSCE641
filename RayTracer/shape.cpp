#include "shape.h"
#include "utility.hpp"

Shape::ShapeType Shape::interpretType(const string &str)
{
    string lowerStr = Utils::toLower(str);
    if( lowerStr == "sphere" )
        return SPHERE;
    else if( lowerStr == "triangle" )
        return TRIANGLE;
    else if( lowerStr == "triangle" )
        return RECTANGLE;
    else if( lowerStr == "triangle" )
        return CUBE;
    else if( lowerStr == "triangle" )
        return POLYGONMESH;
    else
        return UNKNOWN;
}

istream& operator>>(istream& s, Sphere& sp)
{
    s >> sp._center >> sp._radius;
    return s;
}

istream& operator>>(istream& s, Triangle& tr)
{
    for(int i=0;i<3;i++)
        s >> tr._vertices[i];
    return s;
}

istream& operator>>(istream& s, Rectangle& rt)
{
    for(int i=0;i<4;i++)
        s >> rt._vertices[i];
    return s;
}

istream& operator>>(istream& s, Cube& c)
{
    for(int i=0;i<8;i++)
        s >> c._vertices[i];
    return s;
}

istream& operator>>(istream& s, PolygonMesh& p)
{
    // TBA
    throw "not yet implemented.";
    return s;
}

bool Sphere::intersect(const Ray &r, Hit &h)
{
    // get the distance of the ray to the sphere's centre
    double dist = -1.0;

    DblVector3D origToCenter = _center - r.origin();
    DblVector3D dirVec = normalize ( r.dir() );
    DblVector3D crossProduct = GeometryUtils::crossProduct(origToCenter, dirVec);

    // any valid ray should have a length greater than 0
    assert( length(r.dir()) > 0 );

    dist = length( crossProduct );

    if( dist <= _radius )
    {
        // intersects, calculate the hit point
        double part1 = dotProduct( origToCenter, dirVec );       // problematic
        if( part1 < 0 )
            return false;

        double part2 = sqrt(_radius * _radius - dist * dist);
        double t1 = part1 + part2;
        double t2 = part1 - part2;

        double nearT, farT;
        nearT = t2, farT = t1;

        const double ZERO_THRESHOLD = 1e-3;
        double origToHitLength;
        if( nearT <= ZERO_THRESHOLD )
            origToHitLength = farT;
        else
            origToHitLength = nearT;

        if( origToHitLength < h.t())
        {
            DblVector3D diff = dirVec * origToHitLength;
            DblVector3D hitPoint = r.origin() + diff;

            if( _refractionRate > 0 )
                h.set( origToHitLength, _color, hitPoint - _center, true, true, this->_refractionRate );
            else
                h.set( origToHitLength, _color, hitPoint - _center);
        }

        return true;
    }
    else
        return false;
}

bool Sphere::blockTest(const Ray &r, double t)
{
    // get the distance of the ray to the sphere's centre
    double dist = -1.0;

    DblVector3D origToCenter = _center - r.origin();
    DblVector3D dirVec = normalize ( r.dir() );
    DblVector3D crossProduct = GeometryUtils::crossProduct(origToCenter, dirVec);

    // any valid ray should have a length greater than 0
    assert( length(r.dir()) > 0 );

    dist = length( crossProduct );

    if( dist <= _radius )
    {
        // intersects, calculate the hit point
        double part1 = dotProduct( origToCenter, dirVec );
        if( part1 < 0 )
            return false;

        double part2 = sqrt(_radius * _radius - dist * dist);
        double t1 = part1 + part2;
        double t2 = part1 - part2;

        double nearT, farT;
        nearT = t2, farT = t1;

        const double ZERO_THRESHOLD = 1e-3;
        double origToHitLength;
        if( nearT <= ZERO_THRESHOLD )
            origToHitLength = farT;
        else
            origToHitLength = nearT;

        if( origToHitLength < t)
            return true;
        else
            return false;
    }
    else
        return false;
}

bool Rectangle::intersect(const Ray &r, Hit &h)
{
    // rectangle and ray intersection

    // plane parameters
    double A, B, C, D;

    A = _vertices[0].y() * ( _vertices[1].z() - _vertices[2].z())
            + _vertices[1].y() * ( _vertices[2].z() - _vertices[0].z())
            + _vertices[2].y() * ( _vertices[0].z() - _vertices[1].z());

    B = _vertices[0].z() * ( _vertices[1].x() - _vertices[2].x())
            + _vertices[1].z() * ( _vertices[2].x() - _vertices[0].x())
            + _vertices[2].z() * ( _vertices[0].x() - _vertices[1].x());

    C = _vertices[0].x() * ( _vertices[1].y() - _vertices[2].y())
            + _vertices[1].x() * ( _vertices[2].y() - _vertices[0].y())
            + _vertices[2].x() * ( _vertices[0].y() - _vertices[1].y());

    D = - _vertices[0].x() * ( _vertices[1].y() * _vertices[2].z() - _vertices[2].y() * _vertices[1].z() )
            - _vertices[1].x() * ( _vertices[2].y() * _vertices[0].z() - _vertices[0].y() * _vertices[2].z() )
            - _vertices[2].x() * ( _vertices[0].y() * _vertices[1].z() - _vertices[1].y() * _vertices[0].z() );

    DblVector3D dirVec = normalize( r.dir() );
    DblPoint3D origPoint = r.origin();

    double origToHitLength = - ( A * origPoint.x() + B * origPoint.y() + C * origPoint
                                 .z() + D)
            / ( A * dirVec.x() + B * dirVec.y() + C * dirVec.z() );

    const double ZERO_THRESHOLD = 1e-2;
    if( origToHitLength < ZERO_THRESHOLD )
        return false;
    else
    {
        // test if the hit point is within the rectangle
        DblVector3D diff = dirVec;
        diff = diff * origToHitLength;
        DblPoint3D hitPoint = r.origin();
        hitPoint = hitPoint + diff;

        DblVector3D v1 = _vertices[1] - _vertices[0];
        DblVector3D v2 = _vertices[3] - _vertices[0];
        DblVector3D v = hitPoint - _vertices[0];

        double alpha1, alpha2;
        alpha1 = dotProduct(v, v1) / length( v1 );
        alpha2 = dotProduct(v, v2) / length( v2 );

        if( alpha1 >= 0 && alpha1 <= length( v1 )
                && alpha2 >= 0 && alpha2 <= length( v2 ) )
        {
            if( origToHitLength < h.t() )
            {
                if( _refractionRate > 0 )
                    h.set( origToHitLength, _color, _normal, true, true, _refractionRate );
                else
                    h.set( origToHitLength, _color, _normal);
            }

            return true;
        }
        else
            return false;
    }
}

bool Rectangle::blockTest(const Ray &r, double t)
{
    // rectangle and ray intersection

    // plane parameters
    double A, B, C, D;

    A = _vertices[0].y() * ( _vertices[1].z() - _vertices[2].z())
            + _vertices[1].y() * ( _vertices[2].z() - _vertices[0].z())
            + _vertices[2].y() * ( _vertices[0].z() - _vertices[1].z());

    B = _vertices[0].z() * ( _vertices[1].x() - _vertices[2].x())
            + _vertices[1].z() * ( _vertices[2].x() - _vertices[0].x())
            + _vertices[2].z() * ( _vertices[0].x() - _vertices[1].x());

    C = _vertices[0].x() * ( _vertices[1].y() - _vertices[2].y())
            + _vertices[1].x() * ( _vertices[2].y() - _vertices[0].y())
            + _vertices[2].x() * ( _vertices[0].y() - _vertices[1].y());

    D = - _vertices[0].x() * ( _vertices[1].y() * _vertices[2].z() - _vertices[2].y() * _vertices[1].z() )
            - _vertices[1].x() * ( _vertices[2].y() * _vertices[0].z() - _vertices[0].y() * _vertices[2].z() )
            - _vertices[2].x() * ( _vertices[0].y() * _vertices[1].z() - _vertices[1].y() * _vertices[0].z() );

    DblVector3D dirVec = normalize( r.dir() );
    DblPoint3D origPoint = r.origin();

    double origToHitLength = - ( A * origPoint.x() + B * origPoint.y() + C * origPoint
                                 .z() + D)
            / ( A * dirVec.x() + B * dirVec.y() + C * dirVec.z() );

    const double ZERO_THRESHOLD = 1e-2;
    if( origToHitLength < ZERO_THRESHOLD )
        return false;
    else
    {
        // test if the hit point is within the rectangle
        DblVector3D diff = dirVec;
        diff = diff * origToHitLength;
        DblPoint3D hitPoint = r.origin();
        hitPoint = hitPoint + diff;

        DblVector3D v1 = _vertices[1] - _vertices[0];
        DblVector3D v2 = _vertices[3] - _vertices[0];
        DblVector3D v = hitPoint - _vertices[0];

        double alpha1, alpha2;
        alpha1 = dotProduct(v, v1) / length( v1 );
        alpha2 = dotProduct(v, v2) / length( v2 );

        if( alpha1 >= 0 && alpha1 <= length( v1 )
                && alpha2 >= 0 && alpha2 <= length( v2 ) )
        {
            if( origToHitLength < t )
                return true;
            else
                return false;
        }
        else
            return false;
    }
}
