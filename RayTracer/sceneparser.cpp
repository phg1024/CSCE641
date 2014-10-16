#include "sceneparser.h"
#include "shape.h"
#include "scene.h"
#include "camerainfo.h"
#include "lightsource.h"
#include "utility.hpp"

const char SceneParser::commentTag = '#';
const char SceneParser::sectionHeaderTag = '.';

const string SceneParser::sceneInfoSectionTag = "SCENE_INFO";
const string SceneParser::cameraInfoSectionTag = "CAMERA_INFO";
const string SceneParser::lightSourceSectionTag = "LIGHT_SOURCES";
const string SceneParser::shapeSectionTag = "SHAPES";

bool SceneParser::parse(const string filename)
{
    // verify that scene is already binded
    if( !_scene )
        return false;

    // open file and verify validity of file
    ifstream f;
    f.open(filename.c_str(), ios::in);

    if( f.bad() )
        return false;

    // parse the file
    while(!f.eof())
    {
        string line;
        getline(f, line);

        // ignore comments
        if(line.at(0) == commentTag)
            continue;

        if(line.at(0) == sectionHeaderTag)
        {
            istringstream sDef(line.substr(1));
            string sectionName;
            sDef >> sectionName;

            if( sectionName == sceneInfoSectionTag )
            {
                if(!parseSceneInfo(f))
                    return false;
            }

            if( sectionName == cameraInfoSectionTag )
            {
                if(!parseCameraInfo(f))
                    return false;
            }

            if( sectionName == lightSourceSectionTag )
            {
                if(!parseLightSources(f))
                    return false;
            }

            if( sectionName == shapeSectionTag )
            {
                if(!parseShapes(f))
                    return false;
            }
        }
    }

    f.close();
    return true;
}

const string SceneParser::sceneRangeTag = "SCENE_RANGE";
const string SceneParser::boundingBoxTag = "HAS_BOUNDING_BOX";
const string SceneParser::groundTag = "HAS_GROUND";

bool SceneParser::parseSceneInfo(std::ifstream &f)
{
    do
    {
        char c = f.peek();
        if( c == sectionHeaderTag )
            break;

        // get a line from the input file
        string line;
        getline(f, line);

        // get the line tag
        stringstream sline(line);
        string lineTag;
        sline >> lineTag;

        if( lineTag == sceneRangeTag )
        {
            sline >> _scene->_min[0]
                  >> _scene->_max[0]
                  >> _scene->_min[1]
                  >> _scene->_max[1]
                  >> _scene->_min[2]
                  >> _scene->_max[2];
        }

        if( lineTag == boundingBoxTag )
        {
            string val;
            sline >> val;
            _scene->_hasBoundingBox = Utils::truthValue(val);
        }

        if( lineTag == groundTag )
        {
            string val;
            sline >> val;
            _scene->_hasGround = Utils::truthValue(val);
        }
    }while(!f.eof());

    return true;
}

const string SceneParser::cameraPosTag = "CAMERA_POS";
const string SceneParser::cameraDirTag = "CAMERA_DIR";
const string SceneParser::cameraUpTag = "CAMERA_UP";
const string SceneParser::focalLengthTag = "FOCAL_LENGTH";
const string SceneParser::canvasSizeTag = "CANVAS_SIZE";

bool SceneParser::parseCameraInfo(std::ifstream &f)
{
    _scene->_camInfo = new CameraInfo;

    do
    {
        char c = f.peek();
        if( c == sectionHeaderTag )
            break;

        // get a line from the input file
        string line;
        getline(f, line);

        // get the line tag
        stringstream sline(line);
        string lineTag;
        sline >> lineTag;

        if( lineTag == cameraPosTag )
        {
            sline >> _scene->_camInfo->_pos;
        }

        if( lineTag == cameraDirTag )
        {
            sline >> _scene->_camInfo->_dir;
        }

        if( lineTag == cameraUpTag )
        {
            sline >> _scene->_camInfo->_up;
        }

        if( lineTag == focalLengthTag )
        {
            sline >> _scene->_camInfo->_focalLength;
        }

        if( lineTag == canvasSizeTag )
        {
            sline >> _scene->_camInfo->_canvasSize[0]
                  >> _scene->_camInfo->_canvasSize[1];
        }

    }while(!f.eof());

    return true;
}

const string SceneParser::lightSourceNumberTag = "LIGHT_SOURCE_NUMBER";
const string SceneParser::lightTypeTag = "LIGHT_TYPE";
const string SceneParser::lightColorTag = "LIGHT_COLOR";
const string SceneParser::lightPosTag = "LIGHT_POS";

bool SceneParser::parseLightSources(std::ifstream &f)
{
    int lightSourceNumber = 0;
    do
    {
        char c = f.peek();
        if( c == sectionHeaderTag )
            break;

        // get a line from the input file
        string line;
        getline(f, line);

        // get the line tag
        stringstream sline(line);
        string lineTag;
        sline >> lineTag;

        if( lineTag == lightSourceNumberTag )
        {
            sline >> lightSourceNumber;
            _scene->_lightSourceNumber = lightSourceNumber;
            _scene->_lightSources = new LightSource[lightSourceNumber];
            if( !_scene->_lightSources )
                throw "failed to allocate memory for light sources.";

            for(int i=0;i<lightSourceNumber;i++)
            {
                // get a line from the input file
                string cline;
                getline(f, cline);

                // get the line tag
                stringstream csline(cline);
                string clineTag;
                csline >> clineTag;

                if( clineTag == lightTypeTag )
                {
                    string val;
                    csline >> val;
                    LightSource::LightSourceType type = LightSource::interpretType(val);
                    _scene->_lightSources[i]._type = type;
                }

                getline(f, cline);
                csline.clear();
                csline.str(cline);
                csline >> clineTag;

                if( clineTag == lightColorTag )
                {
                    csline >> _scene->_lightSources[i]._color;
                }

                getline(f, cline);
                csline.clear();
                csline.str(cline);
                csline >> clineTag;

                if( clineTag == lightPosTag )
                {
                    csline >> _scene->_lightSources[i]._pos;
                }
            }
        }
    }while(!f.eof());

    return true;
}

const string SceneParser::shapeNumberTag = "SHAPE_NUMBER";
const string SceneParser::shapeTag = "SHAPE";
const string SceneParser::shapeColorTag = "COLOR";
const string SceneParser::shapeParamTag = "PARAMETERS";
const string SceneParser::shapeRefractionRateTag = "REFRACTION_RATE";

bool SceneParser::parseShapes(std::ifstream &f)
{
    do
    {
        char c = f.peek();
        if( c == sectionHeaderTag )
            break;

        // get a line from the input file
        string line;
        getline(f, line);

        // get the line tag
        stringstream sline(line);
        string lineTag;
        sline >> lineTag;

        int shapeNumber;
        if( lineTag == shapeNumberTag )
        {
            sline >> shapeNumber;
            int regularShapeNumber = shapeNumber;
            if( _scene->hasBoundingBox() )
                shapeNumber += 5;

            if( _scene->hasGround() )
                shapeNumber ++;

            _scene->_shapeNumber = shapeNumber;
            _scene->_shapes = new Shape*[shapeNumber];
            if( !_scene->_shapes )
                throw "failed to allocate memory for light sources.";

            for(int i=0;i<regularShapeNumber;i++)
            {
                // get a line from the input file
                string cline;
                getline(f, cline);

                // get the line tag
                stringstream csline(cline);
                string clineTag;
                csline >> clineTag;

                if( clineTag == shapeTag )
                {
                    string val;
                    csline >> val;
                    Shape::ShapeType type = Shape::interpretType(val);
                    switch(type)
                    {
                    case Shape::SPHERE:
                    {
                        Sphere *sp = new Sphere;

                        getline(f, cline);
                        csline.clear();
                        csline.str(cline);
                        csline >> clineTag;

                        if( clineTag == shapeColorTag )
                        {
                            csline >> sp->color();
                        }

                        getline(f, cline);
                        csline.clear();
                        csline.str(cline);
                        csline >> clineTag;

                        if( clineTag == shapeParamTag )
                        {
                            csline >> (*sp);
                        }

                        getline(f, cline);
                        csline.clear();
                        csline.str(cline);
                        csline >> clineTag;

                        if( clineTag == shapeRefractionRateTag )
                        {
                            csline >> sp->refractionRate();
                        }

                        _scene->_shapes[i] = dynamic_cast<Shape*>(sp);

                        break;
                    }
                    default:
                        break;
                    }
                }
            }

            if( _scene->hasBoundingBox() )
            {
                // add bounding box to the scene

                // back
                Rectangle *back = new Rectangle;
                back->color() = DblColor4(0.5, 0.5, 0.5, 1);
                back->normal() = DblVector3D(0, 0, 1);
                back->refractionRate() = 0;
                back->vertex(0) = DblPoint3D( _scene->max(0), _scene->max(1), _scene->min(2));
                back->vertex(1) = DblPoint3D( _scene->min(0), _scene->max(1), _scene->min(2));
                back->vertex(2) = DblPoint3D( _scene->min(0), _scene->min(1), _scene->min(2));
                back->vertex(3) = DblPoint3D( _scene->max(0), _scene->min(1), _scene->min(2));

                _scene->_shapes[regularShapeNumber + 0] = dynamic_cast<Shape*>(back);

                // left
                Rectangle *left = new Rectangle;
                left->color() = DblColor4(0.95, 0.35, 0.35, 1);
                left->normal() = DblVector3D(1, 0, 0);
                left->refractionRate() = 0;
                left->vertex(0) = DblPoint3D( _scene->min(0), _scene->max(1), _scene->min(2));
                left->vertex(1) = DblPoint3D( _scene->min(0), _scene->max(1), _scene->max(2));
                left->vertex(2) = DblPoint3D( _scene->min(0), _scene->min(1), _scene->max(2));
                left->vertex(3) = DblPoint3D( _scene->min(0), _scene->min(1), _scene->min(2));
                _scene->_shapes[regularShapeNumber + 1] = dynamic_cast<Shape*>(left);

                // right
                Rectangle *right = new Rectangle;
                right->color() = DblColor4(0.35, 0.95, 0.35, 1);
                right->normal() = DblVector3D(-1, 0, 0);
                right->refractionRate() = 0;
                right->vertex(0) = DblPoint3D( _scene->max(0), _scene->max(1), _scene->min(2));
                right->vertex(1) = DblPoint3D( _scene->max(0), _scene->max(1), _scene->max(2));
                right->vertex(2) = DblPoint3D( _scene->max(0), _scene->min(1), _scene->max(2));
                right->vertex(3) = DblPoint3D( _scene->max(0), _scene->min(1), _scene->min(2));

                _scene->_shapes[regularShapeNumber + 2] = dynamic_cast<Shape*>(right);

                // bottom
                Rectangle *bottom = new Rectangle;
                bottom->color() = DblColor4(0.25, 0.45, 0.95, 1);
                bottom->normal() = DblVector3D(0, 1, 0);
                bottom->refractionRate() = 0;
                bottom->vertex(0) = DblPoint3D( _scene->min(0), _scene->min(1), _scene->min(2));
                bottom->vertex(1) = DblPoint3D( _scene->min(0), _scene->min(1), _scene->max(2));
                bottom->vertex(2) = DblPoint3D( _scene->max(0), _scene->min(1), _scene->max(2));
                bottom->vertex(3) = DblPoint3D( _scene->max(0), _scene->min(1), _scene->min(2));

                _scene->_shapes[regularShapeNumber + 3] = dynamic_cast<Shape*>(bottom);

                // top
                Rectangle *top = new Rectangle;
                top->color() = DblColor4(0.5, 0.5, 0.5, 1);
                top->normal() = DblVector3D(0, -1, 0);
                top->refractionRate() = 0;
                top->vertex(0) = DblPoint3D( _scene->min(0), _scene->max(1), _scene->min(2));
                top->vertex(1) = DblPoint3D( _scene->min(0), _scene->max(1), _scene->max(2));
                top->vertex(2) = DblPoint3D( _scene->max(0), _scene->max(1), _scene->max(2));
                top->vertex(3) = DblPoint3D( _scene->max(0), _scene->max(1), _scene->min(2));

                _scene->_shapes[regularShapeNumber + 4] = dynamic_cast<Shape*>(top);
            }


            if( _scene->hasGround() )
            {
                // add ground to the scene
                Rectangle *ground = new Rectangle;
                ground->color() = DblColor4(0.95, 0.95, 0.95, 1);
                ground->normal() = DblVector3D(0, 1, 0);
                ground->refractionRate() = 0;

                ground->vertex(0) = DblPoint3D( _scene->min(0), 0, _scene->min(2));
                ground->vertex(1) = DblPoint3D( _scene->min(0), 0, _scene->max(2));
                ground->vertex(2) = DblPoint3D( _scene->max(0), 0, _scene->max(2));
                ground->vertex(3) = DblPoint3D( _scene->max(0), 0, _scene->min(2));
                _scene->_shapes[regularShapeNumber] = dynamic_cast<Shape*>(ground);
            }
        }

    }while(!f.eof());

    return true;
}


