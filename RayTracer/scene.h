#ifndef SCENE_H
#define SCENE_H

#include <stdlib.h>
#include <string>
using namespace std;

#include "camerainfo.h"
#include "lightsource.h"

class SceneParser;
class RayTracer;

class Shape;

class Scene
{
public:
    Scene();
    Scene(const string& filename);
    ~Scene();

    bool intersect(const Ray& r, Hit& h);
    bool blockTest(const Ray& r, double t, bool& translucent, DblColor4& c);

    const CameraInfo& cameraInfo() const
    {
        if( _camInfo )
            return (*_camInfo);
        else
            throw "no camera info specified.";
    }

    CameraInfo& cameraInfo()
    {
        if( _camInfo )
            return (*_camInfo);
        else
            throw "no camera info specified.";
    }

    const size_t& lightSourcesNumber() const
    {
        return _lightSourceNumber;
    }

    const LightSource& lightSource(size_t idx)
    {
        if( idx < _lightSourceNumber )
            return _lightSources[idx];
        else
            throw "light source index out of range!";
    }

    const size_t& shapeNumber() const
    {
        return _shapeNumber;
    }

    const Shape* shape(size_t idx) const
    {
        if( idx < _shapeNumber )
            return _shapes[idx];
        else
            throw "shape index out of range!";
    }

    bool hasGround() { return _hasGround; }
    bool hasBoundingBox() { return _hasBoundingBox; }

    double min(int idx) { return _min[idx]; }
    double max(int idx) { return _max[idx]; }

protected:
    friend class SceneParser;
    friend class RayTracer;

private:
    // scene range
    double _min[3], _max[3];
    bool _hasBoundingBox;
    bool _hasGround;

    CameraInfo* _camInfo;

    size_t _shapeNumber;
    Shape** _shapes;

    size_t _lightSourceNumber;
    LightSource* _lightSources;
};

#endif // SCENE_H
