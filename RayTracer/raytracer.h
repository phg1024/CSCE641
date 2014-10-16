#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "geometryutils.hpp"
using namespace GeometryUtils;

#include "color.hpp"

#include "rgbaimage.h"

#include <cfloat>
#include <QThread>
#include <QApplication>

class Ray
{
public:
    Ray():
        _order(0),
        _refractionRate(0)
    {}

    Ray(const DblVector3D& dir, const DblPoint3D& origin):
        _origin(origin),
        _dir(dir)
    {}
    ~Ray(){}

    Ray(const Ray& other):
        _origin(other._origin),
        _dir(other._dir),
        _order(other._order),
        _refractionRate(other._refractionRate)
    {}

    Ray& operator=(const Ray& other)
    {
        if( this == &other )
            return (*this);
        else
        {
            _origin = (other._origin),
            _dir = (other._dir),
            _order = (other._order),
            _refractionRate = (other._refractionRate);
            return (*this);
        }
    }

    DblPoint3D pointAt( double t )
    {
        // _dir is always normalized
        return _origin + _dir * t;
    }

    DblPoint3D& origin() { return _origin; }
    const DblPoint3D& origin() const { return _origin; }

    DblPoint3D& dir() { return _dir; }
    const DblPoint3D& dir() const { return _dir; }

    size_t& order() { return _order; }
    const size_t& order() const { return _order; }

    double& refractionRate() { return _refractionRate; }
    const double& refractionRate() const { return _refractionRate; }

    friend class RayTracer;

private:
    DblPoint3D _origin;
    DblVector3D _dir;
    size_t _order;
    double _refractionRate;
};

class Hit
{
public:
    Hit():_t(DBL_MAX), _color(0, 0, 0, 0),
        _normal(0, 0, 0),
        _reflected(false),
        _refracted(false),
        _refractionRate(0)
    {}
    ~Hit(){}

    Hit(const Hit& other):
        _t(other._t),
        _color(other._color),
        _normal(other._normal),
        _reflected(other._reflected),
        _refracted(other._refracted),
        _refractionRate(other._refractionRate)
    {}

    Hit& operator=(const Hit& other)
    {
        if( this == &other )
        {
            return (*this);
        }
        else
        {
            _t = (other._t);
            _color = (other._color);
            _normal = (other._normal);
            _reflected = (other._reflected);
            _refracted = (other._refracted);
            _refractionRate = (other._refractionRate);
            return (*this);
        }
    }

    void set( double t, const DblColor4& c,
              const DblVector3D& normal,
              bool refl = true,
              bool refr = false,
              double refrRate = 0)
    {
        _t = t;
        _color = c;
        _normal = normal;
        _reflected = refl;
        _refracted = refr;
        _refractionRate = refrRate;
    }

    double& t() { return _t; }
    const double& t() const {return _t; }

    DblColor4& color() { return _color; }
    const DblColor4& color() const { return _color; }

    bool& reflected() { return _reflected; }
    const bool& reflected() const { return _reflected; }

    bool& refracted() { return _refracted; }
    const bool& refracted() const { return _refracted; }

    DblVector3D& normal() { return _normal; }
    const DblVector3D& normal() const { return _normal; }

    double& refractionRate() { return _refractionRate; }
    const double& refractionRate() const { return _refractionRate; }

    friend class RayTracer;

private:
    double _t;
    DblColor4 _color;

    // hit point normal
    DblVector3D _normal;

    bool _reflected;
    bool _refracted;
    double _refractionRate;
};

class Canvas
{
public:
    Canvas():img(0){}
    Canvas(size_t w, size_t h):
        img(new RGBAImage(w, h, 0.0))
    {}

    ~Canvas()
    {
        if( img )
            delete img;
    }

    friend class RayTracer;

private:
    RGBAImage* img;
};

// threaded ray tracing
class RayTracingThread : public QThread
{

};

class Scene;

class RayTracer : public QObject
{
    Q_OBJECT
public:
    RayTracer();

    void setSize(int w, int h);
    void bindScene(Scene *s);
    void execute();

    const RGBAImage& result() { return _renderImage; }

signals:
    void sig_progress(double);

protected:
    void rayTracing();
    void rayTracing_Threaded();

    bool trace( const Ray& r, Hit& h );

    DblColor4 evaluateLighting( const Ray& r, const Hit& h);
    Ray reflect( const Ray& r, const Hit& h );
    Ray refract( const Ray& r, const Hit& h );

private:
    Scene* _scene;
    Canvas* _canvas;
    bool _threaded;
    bool _isMSAAEnabled;
    int _MSAASampleNumber;
    DblVector2D _shiftVector[8];

    DblColor4 _bgColor;

    double _directLightingFactor;
    double _reflectionFactor;
    double _refractionFactor;

    size_t _maxIterations;

    RGBAImage _renderImage;
};

#endif // RAYTRACER_H
