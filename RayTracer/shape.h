#ifndef SHAPE_H
#define SHAPE_H

#include <string>
#include <iostream>
using namespace std;

#include "geometryutils.hpp"
using namespace GeometryUtils;

#include "color.hpp"

#include "raytracer.h"

class Shape
{
public:
    enum ShapeType
    {
        TRIANGLE,
        RECTANGLE,
        SPHERE,
        CUBE,
        POLYGONMESH,
        UNKNOWN
    };

    Shape(){}
    Shape(ShapeType t):
        _type(t)
    {}
    Shape(const Shape& other):
        _refractionRate(other._refractionRate),
        _type(other._type),
        _color(other._color)
    {}
    virtual ~Shape(){}

    virtual bool intersect(const Ray &r, Hit &h) = 0;
    virtual bool blockTest(const Ray &r, double t) = 0;

    const ShapeType& type() const { return _type; }
    static ShapeType interpretType(const string&);

public:
    double& refractionRate(){ return _refractionRate; }
    const double& refractionRate() const{ return _refractionRate; }

    const DblColor4& color() const { return _color; }
    DblColor4& color() {return _color;}

protected:
    double _refractionRate;
    ShapeType _type;
    DblColor4 _color;
};

class Triangle : public Shape
{
public:
    Triangle():Shape(TRIANGLE){}
    Triangle(const Triangle& other):
        Shape(dynamic_cast<const Shape&>(other)),
        _normal(other._normal)
    {
        for(int i=0;i<3;i++)
            _vertices[i] = other._vertices[i];
    }
    ~Triangle(){}

    friend istream& operator>>(istream&, Triangle&);

private:
    DblPoint3D _vertices[3];
    DblVector3D _normal;
};

class Rectangle : public Shape
{
public:
    Rectangle():Shape(RECTANGLE){}
    Rectangle(const Rectangle& other):
        Shape(dynamic_cast<const Shape&>(other)),
        _normal(other._normal)
    {
        for(int i=0;i<4;i++)
            _vertices[i] = other._vertices[i];
    }
    virtual ~Rectangle(){}

    virtual bool intersect(const Ray &r, Hit &h);
    virtual bool blockTest(const Ray &r, double t);

    const DblPoint3D& vertex( size_t idx ) const
    {
        if( idx < 4)
            return _vertices[idx];
        else
            throw "index out of range.";
    }

    DblPoint3D& vertex( size_t idx )
    {
        if( idx < 4)
            return _vertices[idx];
        else
            throw "index out of range.";
    }

    const DblVector3D& normal() const { return _normal; }
    DblVector3D& normal() { return _normal; }

    friend istream& operator>>(istream&, Rectangle&);

private:
    DblPoint3D _vertices[4];
    DblVector3D _normal;
};

class Sphere : public Shape
{
public:
    Sphere():Shape(SPHERE){}
    Sphere(const Sphere& other):
        Shape(dynamic_cast<const Shape&>(other)),
        _center(other._center),
        _radius(other._radius)
    {}

    virtual ~Sphere(){}

    virtual bool intersect(const Ray &r, Hit &h);
    virtual bool blockTest(const Ray &r, double t);

    friend istream& operator>>(istream&, Sphere&);

    const DblPoint3D& center() const { return _center; }
    DblPoint3D& center() { return _center; }

    const double& radius() const { return _radius; }
    double& radius() { return _radius; }

private:
    DblPoint3D _center;
    double _radius;
};

class Cube : public Shape
{
public:
    Cube():Shape(CUBE){}
    Cube(const Cube& other):
        Shape(dynamic_cast<const Shape&>(other))
    {
        for(int i=0;i<8;i++)
            _vertices[i] = other._vertices[i];
    }
    ~Cube(){}

    friend istream& operator>>(istream&, Cube&);

private:
    DblPoint3D _vertices[8];
};

class PolygonMesh : public Shape
{
public:
    PolygonMesh():Shape(POLYGONMESH){}
    ~PolygonMesh(){}

    friend istream& operator>>(istream&, PolygonMesh&);

private:
    // a list of polygons
};

#endif // SHAPE_H
