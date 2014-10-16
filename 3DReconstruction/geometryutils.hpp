#ifndef GEOMETRY_UTILS_H
#define GEOMETRY_UTILS_H

#include "mathutil.hpp"

using namespace MathUtils;

#include <cstdlib>
#include <cassert>
#include <cmath>
#include <list>
using namespace std;

namespace GeometryUtils
{
template <typename T>
class Point2D
{
public:
    Point2D():
        _x(0), _y(0){}
    Point2D(T x, T y):
        _x(x), _y(y){}
    Point2D(const Point2D& other):
        _x(other._x), _y(other._y){}

    const T& x() const {return _x;}
    T& x() {return _x;}
    const T& y() const {return _y;}
    T& y() {return _y;}

    Point2D& operator=(const Point2D& op)
    {
        if(this == &op)
        {
            return (*this);
        }
        else
        {
            _x = op.x();
            _y = op.y();
            return (*this);
        }
    }

    DblVector toVector()
    {
        DblVector v(2);
        v(0) = _x, v(1) = _y;
        return v;
    }

    const Point2D operator+(const Point2D& op) const
    {
        return Point2D(_x + op.x(), _y + op.y());
    }

    const Point2D operator-(const Point2D& op) const
    {
        return Point2D(_x - op.x(), _y - op.y());
    }

    const Point2D operator*(const double& factor) const
    {
        return Point2D(_x * factor, _y * factor);
    }

    const Point2D operator/(const double& factor) const
    {
        if(factor == 0)
            return;
        double invFactor = 1.0 / factor;
        return (*this) * invFactor;
    }

protected:
    T _x, _y;
};

template <typename T>
class Point3D : public Point2D<T>
{
public:
    Point3D(T x, T y, T z):
        Point2D<T>(x, y), _z(z){}
    Point3D():
        Point2D<T>(),_z(0){}
    Point3D(const Point2D<T>& p, T z = 0):
        Point2D<T>(p), _z(z){}
    Point3D(const Point3D<T> &other):
        Point2D<T>(other._x, other._y), _z(other._z){}

    const T& z() const {return _z;}
    T& z() {return _z;}

    DblVector toVector()
    {
        DblVector v(3);
        v(0) = Point2D<T>::_x, v(1) = Point2D<T>::_y, v(2) = _z;
        return v;
    }

    static Point3D fromVector(const DblVector& v)
    {
        assert(v.length() == 3);
        Point3D p;
        p._x = v(0), p._y = v(1), p._z = v(2);
        return p;
    }

    Point3D& operator=(const Point3D& op)
    {
        if(this == &op)
        {
            return (*this);
        }
        else
        {
            Point2D<T>::_x = op.x();
            Point2D<T>::_y = op.y();
            _z = op.z();
            return (*this);
        }
    }

    const Point3D operator+(const Point3D& op) const
    {
        return Point3D(Point2D<T>::_x + op.x(),Point2D<T>::_y + op.y(), _z + op.z());
    }

    const Point3D operator-(const Point3D& op) const
    {
        return Point3D(Point2D<T>::_x + op.x(), Point2D<T>::_y + op.y(), _z - op.z());
    }

    const Point3D operator*(const double& factor) const
    {
        return Point3D(Point2D<T>::_x * factor,Point2D<T>::_y * factor, _z * factor);
    }

    const Point3D operator/(const double& factor) const
    {
        if(factor == 0)
            return;
        double invFactor = 1.0 / factor;
        return (*this) * invFactor;
    }

protected:
    T _z;
};

template <typename T>
class Point4D : public Point3D<T>
{
public:
    Point4D(T x, T y, T z, T w):
        Point3D<T>(x, y, z), _w(w){}
    Point4D():
        Point3D<T>(),_w(0){}
    Point4D(const Point3D<T>& p, T w = 0):
        Point3D<T>(p), _w(w){}
    Point4D(const Point4D<T> &other):
        Point3D<T>(other._x, other._y, other._z), _w(other._w){}

    const T& w() const {return _w;}
    T& w() {return _w;}

    DblVector toVector()
    {
        DblVector v(4);
        v(0) = Point3D<T>::_x, v(1) = Point3D<T>::_y, v(2) = Point3D<T>::_z, v(3) = _w;
        return v;
    }

    static Point4D fromVector(const DblVector& v)
    {
        assert(v.length() == 4);
        Point4D p;
        p._x = v(0), p._y = v(1), p._z = v(2), p._w = v(3);
        return p;
    }

    Point4D& operator=(const Point4D& op)
    {
        if(this == &op)
        {
            return (*this);
        }
        else
        {
            Point3D<T>::_x = op.x();
            Point3D<T>::_y = op.y();
            Point3D<T>::_z = op.z();
            _w = op.w();
            return (*this);
        }
    }

    const Point4D operator+(const Point4D& op) const
    {
        return Point4D(Point3D<T>::_x + op.x(),Point3D<T>::_y + op.y(), Point3D<T>::_z + op.z(), _w + op.w());
    }

    const Point4D operator-(const Point4D& op) const
    {
        return Point4D(Point3D<T>::_x + op.x(), Point3D<T>::_y + op.y(), Point3D<T>::_z - op.z(), _w - op.w());
    }

    const Point4D operator*(const double& factor) const
    {
        return Point4D(Point3D<T>::_x * factor,Point3D<T>::_y * factor, Point3D<T>::_z * factor, _w * factor);
    }

    const Point4D operator/(const double& factor) const
    {
        if(factor == 0)
            return;
        double invFactor = 1.0 / factor;
        return (*this) * invFactor;
    }

protected:
    T _w;
};

template <typename T>
struct Polygon
{
    list<Point2D<T> > points;
    bool isClockWise;
};

typedef Point2D<double> DblPoint2D;
typedef Point3D<double> DblPoint3D;
typedef Point4D<double> DblPoint4D;
typedef DblPoint2D DblVector2D;
typedef DblPoint3D DblVector3D;
typedef DblPoint4D DblVector4D;
typedef Polygon<double> DblPolygon;

void assignPolygonOrientation(DblPolygon& polygon);
inline double dotProduct(const DblVector2D& v1, const DblVector2D& v2);
inline double dotProduct(const DblVector3D& v1, const DblVector3D& v2);
inline DblVector3D crossProduct(const DblVector3D& v1, const DblVector3D& v2);
inline double crossProduct(const DblVector2D& v1, const DblVector2D& v2);
inline DblPoint3D matrixMulPoint(const DblMatrix& mat, const DblPoint3D& p);
}

#endif //GEOMETRY_UTILS_H
