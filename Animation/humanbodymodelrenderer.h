#ifndef HUMANBODYMODELRENDERER_H
#define HUMANBODYMODELRENDERER_H

#include "GL/glew.h"
#include "GL/glut.h"

class HumanBodyModel;
class BoneSegment;
class FrameParameters;
using GeometryUtils::DblVector3D;

class HumanBodyModelRenderer
{
public:
    HumanBodyModelRenderer();
    ~HumanBodyModelRenderer();

    void bindModel( HumanBodyModel *m )
    {
       	_m = m;
	if( _m )
	    generateDisplayLists();
    }
    void draw(int frameIdx = -1);
    void draw(const FrameParameters& pose, const DblVector3D& displacement);

protected:
    void drawBone(BoneSegment* bone);
    void drawBone(BoneSegment* bone, const FrameParameters&);
    void drawAxis();
    void generateDisplayLists();

public:
    static void drawTube(double x0, double y0, double z0,
                  double x1, double y1, double z1,
                  double radius, int layer = 20, int slice = 20);

private:
    HumanBodyModel* _m;
    int _frameIdx;
    GLuint* _dplist;
    int _dplistSize;
};

#endif // HUMANBODYMODELRENDERER_H
