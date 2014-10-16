#ifndef SCENEPARSER_H
#define SCENEPARSER_H

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

/*
 * Scene file format:
 * # scene info
 * SCENE minX, maxX, minY, maxY, minZ, maxZ
 * HAS_BOUNDING_BOX false
 * HAS_GROUND true
 * # camera info
 * CAMERA_POS 0 0 5.0
 * CAMERA_DIR 0 0 -1
 * CAMERA_UP 0 1 0
 * FOCAL_LENGTH 2.5
 * CANVAS_SIZE 2.0 1.5
 * # light source info
 * LIGHT_TYPE POINT
 * LIGHT_POS -3.0 3.0 3.0
 * ...
 * # shape info
 * SHAPE shapeName
 * PARAMETERS param0, param1
 * REFRACTION_RATE value
 * ...
 */

class Scene;

class SceneParser
{
public:
    SceneParser():_scene(0){}
    ~SceneParser(){}
    void bindScene(Scene* s)
    {
        _scene = s;
    }

    bool parse(const string filename);

public:
    static const char commentTag;
    static const char sectionHeaderTag;

    static const string sceneInfoSectionTag;
    static const string cameraInfoSectionTag;
    static const string lightSourceSectionTag;
    static const string shapeSectionTag;

    static const string sceneRangeTag;
    static const string boundingBoxTag;
    static const string groundTag;

    static const string cameraPosTag;
    static const string cameraDirTag;
    static const string cameraUpTag;
    static const string focalLengthTag;
    static const string canvasSizeTag;

    static const string lightSourceNumberTag;
    static const string lightTypeTag;
    static const string lightColorTag;
    static const string lightPosTag;

    static const string shapeNumberTag;
    static const string shapeTag;
    static const string shapeColorTag;
    static const string shapeParamTag;
    static const string shapeRefractionRateTag;

protected:
    bool parseSceneInfo(ifstream &f);
    bool parseCameraInfo(ifstream &f);
    bool parseLightSources(ifstream &f);
    bool parseShapes(ifstream &f);

private:
    Scene *_scene;
};

#endif // SCENEPARSER_H
