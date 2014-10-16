#ifndef GEOMETRY_UTILS_H
#define GEOMETRY_UTILS_H

#include "mathutil.hpp"

using namespace MathUtils;

#include <cstdlib>
#include <cassert>
#include <cmath>
#include <iostream>
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

    Point2D operator+(const Point2D& op)
    {
        return Point2D(_x + op.x(), _y + op.y());
    }

    template <typename PT>
    friend Point2D<PT> operator+(const Point2D<PT>& lhs, const Point2D<PT>& rhs);

    Point2D operator-(const Point2D& op)
    {
        return Point2D(_x - op.x(), _y - op.y());
    }

    template <typename PT>
    friend Point2D<PT> operator-(const Point2D& lhs, const Point2D& rhs);

    Point2D operator*(const double& factor)
    {
        return Point2D(_x * factor, _y * factor);
    }

    Point2D operator/(const double& factor)
    {
        if(factor == 0)
            return Point2D(0, 0);
        double invFactor = 1.0 / factor;
        return (*this) * invFactor;
    }

    template <typename PT>
    friend Point2D<PT> operator*(const double& factor, const Point2D<PT>& p);

    template <typename PT>
    friend istream& operator>>(istream& s, Point2D<PT>& p);

    template <typename PT>
    friend ostream& operator<<(ostream& s, Point2D<PT>& p);

protected:
    T _x, _y;
};

template <typename PT>
Point2D<PT> operator*(const double& factor, const Point2D<PT>& p)
{
    return (p * factor);
}

template <typename PT>
Point2D<PT> operator+(const Point2D<PT>& p1, const Point2D<PT>& p2)
{
    Point2D<PT> p = p1;
    return p + p2;
}

template <typename PT>
Point2D<PT> operator-(const Point2D<PT>& p1, const Point2D<PT>& p2)
{
    Point2D<PT> p = p1;
    return p - p2;
}

template <typename PT>
istream& operator>>(istream& s, Point2D<PT>& p)
{
    s >> p.x() >> p.y();
    return s;
}

template <typename PT>
ostream& operator<<(ostream& s, Point2D<PT>& p)
{
    s << p.x() << "\t" << p.y();
    return s;
}

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

    Point3D operator+(const Point3D& op)
    {
        return Point3D(Point2D<T>::_x + op.x(),Point2D<T>::_y + op.y(), _z + op.z());
    }

    template <typename PT>
    friend Point3D<PT> operator+(const Point3D<PT>& lhs, const Point3D<PT>& rhs);

    Point3D operator-(const Point3D& op)
    {
        return Point3D(Point2D<T>::_x - op.x(), Point2D<T>::_y - op.y(), _z - op.z());
    }

    template <typename PT>
    friend Point3D<PT> operator+(const Point3D<PT>& lhs, const Point3D<PT>& rhs);

    Point3D operator*(const double& factor)
    {
        return Point3D(Point2D<T>::_x * factor,Point2D<T>::_y * factor, _z * factor);
    }

    Point3D operator/(const double& factor)
    {
        if(factor == 0)
            return Point3D(0, 0, 0);
        double invFactor = 1.0 / factor;
        return (*this) * invFactor;
    }

    template <typename PT>
    friend Point3D<PT> operator*(const double& factor, const Point3D<PT>& p);

    template <typename PT>
    friend Point3D<PT> operator*(const Point3D<PT>& p, const double& factor);

    Point2D<T> xy()
    {
        return Point2D<T>(this->_x, this->_y);
    }

    template <typename PT>
    friend istream& operator>>(istream& s, Point3D<PT>& p);

    template <typename PT>
    friend ostream& operator<<(ostream& s, Point3D<PT>& p);

protected:
    T _z;
};

template <typename PT>
Point3D<PT> operator*(const double& factor, const Point3D<PT>& p)
{
    Point3D<PT> fp = p;
    return (fp * factor);
}

template <typename PT>
Point3D<PT> operator*(const Point3D<PT>& p, const double& factor)
{
    Point3D<PT> fp = p;
    return (fp * factor);
}

template <typename PT>
Point3D<PT> operator+(const Point3D<PT>& p1, const Point3D<PT>& p2)
{
    Point3D<PT> p = p1;
    return p + p2;
}

template <typename PT>
Point3D<PT> operator-(const Point3D<PT>& p1, const Point3D<PT>& p2)
{
    Point3D<PT> p = p1;
    return p - p2;
}

template <typename PT>
istream& operator>>(istream& s, Point3D<PT>& p)
{
    s >> p.x() >> p.y() >> p.z();
    return s;
}

template <typename PT>
ostream& operator<<(ostream& s, Point3D<PT>& p)
{
    s << p.x() << "\t"
      << p.y() << "\t"
      << p.z();
    return s;
}

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

    Point4D operator+(const Point4D& op)
    {
        return Point4D(Point3D<T>::_x + op.x(),Point3D<T>::_y + op.y(), Point3D<T>::_z + op.z(), _w + op.w());
    }

    Point4D operator-(const Point4D& op)
    {
        return Point4D(Point3D<T>::_x - op.x(), Point3D<T>::_y - op.y(), Point3D<T>::_z - op.z(), _w - op.w());
    }

    Point4D operator*(const double& factor)
    {
        return Point4D(Point3D<T>::_x * factor,Point3D<T>::_y * factor, Point3D<T>::_z * factor, _w * factor);
    }

    template <typename PT>
    friend Point4D<PT> operator*(const double& factor, const Point4D<PT>& p);

    template <typename MT, typename PT>
    friend Point4D<PT> operator*(const Matrix<MT>& m, const Point4D<PT>& p);

    template <typename PT>
    friend istream& operator>>(istream& s, Point4D<PT>& p);

    template <typename PT>
    friend ostream& operator<<(ostream& s, Point4D<PT>& p);

    Point4D operator/(const double& factor)
    {
        if(factor == 0)
            return Point4D(0, 0, 0, 0);
        double invFactor = 1.0 / factor;
        return (*this) * invFactor;
    }

    Point3D<T> xyz()
    {
        return Point3D<T>(this->_x,
                          this->_y,
                          this->_z);
    }

protected:
    T _w;
};

template <typename PT>
istream& operator>>(istream& s, Point4D<PT>& p)
{
    s >> p.x() >> p.y() >> p.z() >> p.w();
    return s;
}

template <typename PT>
ostream& operator<<(ostream& s, Point4D<PT>& p)
{
    s << p.x() << p.y() << p.z() << p.w();
    return s;
}

template <typename PT>
Point4D<PT> operator*(const double& factor, const Point4D<PT>& p)
{
    return (p * factor);
}


template <typename MT, typename PT>
Point4D<PT> operator*(const Matrix<MT>& m, const Point4D<PT>& p)
{
    if(m.cols() == 4)
    {
        Point4D<PT> t_p;
        Vector<PT> vp(4);
        vp(0) = p.x(), vp(1) = p.y(), vp(2) = p.z(), vp(3) = p.w();

        Vector<PT> t_vp = m * vp;
        t_p.x() = t_vp(0), t_p.y() = t_vp(1), t_p.z() = t_vp(2), t_p.w() = t_vp(3);

        return t_p;
    }
    else
        throw "matrix and vector dimension mismatch!";
}

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
double dotProduct(const DblVector2D& v1, const DblVector2D& v2);
double dotProduct(const DblVector3D& v1, const DblVector3D& v2);
DblVector3D crossProduct(const DblVector3D& v1, const DblVector3D& v2);
double crossProduct(const DblVector2D& v1, const DblVector2D& v2);
DblPoint3D matrixMulPoint(const DblMatrix& mat, const DblPoint3D& p);
DblVector2D normalize(const DblVector2D& v);
DblVector3D normalize(const DblVector3D& v);
double length(const DblVector2D& v);
double length(const DblVector3D& v);
double distance(const DblPoint2D& p1, const DblPoint2D& p2);
double distance(const DblPoint3D& p1, const DblPoint3D& p2);
double squareDistance(const DblPoint2D& p1, const DblPoint2D& p2);
double squareDistance(const DblPoint3D& p1, const DblPoint3D& p2);
}

#endif //GEOMETRY_UTILS_H
