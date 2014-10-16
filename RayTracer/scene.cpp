#include "scene.h"
#include "shape.h"
#include "sceneparser.h"

Scene::Scene():
    _camInfo(0),
    _shapes(0),
    _lightSources(0)
{
}

Scene::Scene(const string &filename)
{
    SceneParser parser;
    parser.bindScene(this);

    if( !parser.parse(filename) )
        throw "failed to parse scene file!";
}

Scene::~Scene()
{
    if(_camInfo)
        delete[] _camInfo;
    if(_shapes)
        delete[] _shapes;
    if(_lightSources)
        delete[] _lightSources;
}

bool Scene::intersect(const Ray &r, Hit &h)
{
    bool intersectFlag = false;
    for(size_t i = 0; i < _shapeNumber; i++)
    {
        if( _shapes[i] )
        {
            intersectFlag |= _shapes[i]->intersect(r, h);
        }
    }

    return intersectFlag;
}

bool Scene::blockTest(const Ray &r, double t, bool& translucent, DblColor4 &c)
{    
    bool blockFlag = false;
    translucent = true;
    c = DblColor4(c.r(), c.g(), c.b(), 0);
    for(size_t i = 0; i < _shapeNumber; i++)
    {
        if( _shapes[i] )
        {
            if( _shapes[i]->blockTest(r, t) )
            {
                translucent &= (_shapes[i]->refractionRate() > 0);
                if( translucent )
                {
                    c = DblColor4::blend(_shapes[i]->color(), c);
                }
                blockFlag = true;
            }
        }
    }

    c.a() = 1.0 - c.a();

    return blockFlag;
}
