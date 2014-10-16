#include "humanbodymodel.h"
#include "humanbodymodelrenderer.h"

#include <iostream>

#include "geometryutils.hpp"

HumanBodyModelRenderer::HumanBodyModelRenderer():
    _m(0),
    _frameIdx(-1),
    _dplist(0)
{
}

HumanBodyModelRenderer::~HumanBodyModelRenderer()
{
    if( _dplist )
	delete[] _dplist;
}

void HumanBodyModelRenderer::draw(int frameIdx)
{
    if( !_m )
    {
        cout << "no model loaded." << endl;
        return;
    }

    _frameIdx = frameIdx;

    // render the model
    if( _frameIdx < 0 )
    {
        glPushMatrix();
        // draw the default pose
        const float globalScaleFactor = _m->scale();
        glScalef(globalScaleFactor, globalScaleFactor, globalScaleFactor);
        BoneSegment* root = _m->_boneSegments[0];
        glTranslatef(root->_pos.x(), root->_pos.y(), root->_pos.z());
        glRotatef(root->_orient[0], 1, 0, 0);
        glRotatef(root->_orient[1], 0, 1, 0);
        glRotatef(root->_orient[2], 0, 0, 1);

        // render the model using DFS
        drawBone( root );

        glRotatef(-root->_orient[2], 0, 0, 1);
        glRotatef(-root->_orient[1], 0, 1, 0);
        glRotatef(-root->_orient[0], 1, 0, 0);
        glTranslatef(-root->_pos.x(), -root->_pos.y(), -root->_pos.z());

        glPopMatrix();
    }
    else
    {
        // find the pose parameters and render the pose
        glPushMatrix();
        // draw the default pose
        const float globalScaleFactor = _m->scale();
        glScalef(globalScaleFactor, globalScaleFactor, globalScaleFactor);
        BoneSegment* root = _m->_boneSegments[0];
        glTranslatef(root->_pos.x(), root->_pos.y(), root->_pos.z());
        glRotatef(root->_orient[0], 1, 0, 0);
        glRotatef(root->_orient[1], 0, 1, 0);
        glRotatef(root->_orient[2], 0, 0, 1);

        FrameParameters& pose = _m->_frames[frameIdx];

        // apply the root pose
        BoneSegmentParameter& boneParam = pose._boneParams[0];
        // root must have 6 parameters, tx ty tz and rx ry rz
        assert( boneParam._numParams == 6 );
        glTranslatef(boneParam._params[0],
                     boneParam._params[1],
                     boneParam._params[2]);

        glRotatef(boneParam._params[5], 0, 0, 1);
        glRotatef(boneParam._params[4], 0, 1, 0);
        glRotatef(boneParam._params[3], 1, 0, 0);

        // render the model using DFS
        drawBone( root );

        glRotatef(-root->_orient[2], 0, 0, 1);
        glRotatef(-root->_orient[1], 0, 1, 0);
        glRotatef(-root->_orient[0], 1, 0, 0);
        glTranslatef(-root->_pos.x(), -root->_pos.y(), -root->_pos.z());

        glPopMatrix();
    }
}

void HumanBodyModelRenderer::draw(const FrameParameters& posture, const DblVector3D& displacement)
{
    if( posture._boneCount <= 0 )
        return;

    glPushMatrix();
    // draw the default pose
    const float globalScaleFactor = _m->scale();
    glScalef(globalScaleFactor, globalScaleFactor, globalScaleFactor);
    glTranslatef(displacement.x(), displacement.y(), displacement.z());
    BoneSegment* root = _m->_boneSegments[0];
    glTranslatef(root->_pos.x(), root->_pos.y(), root->_pos.z());
    glRotatef(root->_orient[0], 1, 0, 0);
    glRotatef(root->_orient[1], 0, 1, 0);
    glRotatef(root->_orient[2], 0, 0, 1);

    const FrameParameters& pose = posture;

    // apply the root pose
    BoneSegmentParameter& boneParam = pose._boneParams[0];
    // root must have 6 parameters, tx ty tz and rx ry rz
    assert( boneParam._numParams == 6 );
    glTranslatef(boneParam._params[0],
                 boneParam._params[1],
                 boneParam._params[2]);

    glRotatef(boneParam._params[5], 0, 0, 1);
    glRotatef(boneParam._params[4], 0, 1, 0);
    glRotatef(boneParam._params[3], 1, 0, 0);

    // render the model using DFS
    drawBone( root, posture );

    glRotatef(-root->_orient[2], 0, 0, 1);
    glRotatef(-root->_orient[1], 0, 1, 0);
    glRotatef(-root->_orient[0], 1, 0, 0);
    glTranslatef(-root->_pos.x(), -root->_pos.y(), -root->_pos.z());

    glPopMatrix();
}

void HumanBodyModelRenderer::drawBone(BoneSegment *bone, const FrameParameters& posture)
{
    // draw the bone, then draw all its children recursively
    // ignore the root node
    glPushMatrix();
    if( bone->_id > 0 )
    {
        glMultMatrixd(bone->_rot_parent_to_self.rawData());

        if( 0 )
            drawAxis();

        double endX, endY, endZ;
        endX = bone->_length * bone->_dir.x();
        endY = bone->_length * bone->_dir.y();
        endZ = bone->_length * bone->_dir.z();

        glPushMatrix();
        if( _frameIdx >= 0)
        {
            const FrameParameters& pose = posture;

            // apply the root pose
            BoneSegmentParameter& boneParam = pose._boneParams[bone->_id];

            if( bone->_dof != boneParam._numParams )
                throw "amc file contains error! cannot display animation!";

            for(int i=bone->_dof - 1;i>=0;i--)
            {
                string dofname = bone->_dofName[i];
#if 1
                if(dofname == "rx") glRotatef(boneParam._params[i], 1., 0., 0.);
                if(dofname == "ry") glRotatef(boneParam._params[i], 0., 1,  0.);
                if(dofname == "rz") glRotatef(boneParam._params[i], 0., 0., 1.);
#else
                // for testing purpose
                if(dofname == "rx") glMultMatrixd(makeXRotationMatrix_deg(boneParam._params[i]).transposed().rawData());
                if(dofname == "ry") glMultMatrixd(makeYRotationMatrix_deg(boneParam._params[i]).transposed().rawData());
                if(dofname == "rz") glMultMatrixd(makeZRotationMatrix_deg(boneParam._params[i]).transposed().rawData());
#endif

            }
        }

#if 0
        glPushMatrix();

        // draw the joint
        glColor4f(1, 0, 0, 1);
        GLfloat joint_diffuse[] = {1, 0.75, 0.65, 1.0};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, joint_diffuse);
        double radius = powf(bone->_length / 2.0, 0.25);
        glutSolidSphere(radius, 20, 20);

        // draw the bone
        glColor4f(0.65, 0.75, 1.0, 1.0);
        GLfloat bone_diffuse[] = {0.7, 1.0, 0.85, 1.0};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, bone_diffuse);
        drawTube(0, 0, 0, endX, endY, endZ, 0.75 * radius, 40, 40);

        glPopMatrix();
#else
        glCallList( _dplist[bone->_id] );
#endif

        glTranslatef(endX, endY, endZ);
    }

    for(int i=0;i<bone->_numChildren;i++)
    {
        BoneSegment* child = _m->_boneSegments[bone->_children[i]];
        drawBone( child, posture );
    }

    glPopMatrix();
    glPopMatrix();
}

void HumanBodyModelRenderer::drawBone(BoneSegment *bone)
{
    // draw the bone, then draw all its children recursively
    // ignore the root node
    glPushMatrix();
    if( bone->_id > 0 )
    {
        glMultMatrixd(bone->_rot_parent_to_self.rawData());

        if( 0 )
            drawAxis();

        double endX, endY, endZ;
        endX = bone->_length * bone->_dir.x();
        endY = bone->_length * bone->_dir.y();
        endZ = bone->_length * bone->_dir.z();

        glPushMatrix();
        if( _frameIdx >= 0)
        {
            FrameParameters& pose = _m->_frames[_frameIdx];

            // apply the root pose
            BoneSegmentParameter& boneParam = pose._boneParams[bone->_id];

            if( bone->_dof != boneParam._numParams )
                throw "amc file contains error! cannot display animation!";

            for(int i=bone->_dof - 1;i>=0;i--)
            {
                string dofname = bone->_dofName[i];
#if 1
                if(dofname == "rx") glRotatef(boneParam._params[i], 1., 0., 0.);
                if(dofname == "ry") glRotatef(boneParam._params[i], 0., 1,  0.);
                if(dofname == "rz") glRotatef(boneParam._params[i], 0., 0., 1.);
#else
                // for testing purpose
                if(dofname == "rx") glMultMatrixd(makeXRotationMatrix_deg(boneParam._params[i]).transposed().rawData());
                if(dofname == "ry") glMultMatrixd(makeYRotationMatrix_deg(boneParam._params[i]).transposed().rawData());
                if(dofname == "rz") glMultMatrixd(makeZRotationMatrix_deg(boneParam._params[i]).transposed().rawData());
#endif

            }
        }

#if 0	
        glPushMatrix();

        // draw the joint
        glColor4f(1, 0, 0, 1);
        GLfloat joint_diffuse[] = {1, 0.75, 0.65, 1.0};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, joint_diffuse);
	double radius = powf(bone->_length / 2.0, 0.25);
        glutSolidSphere(radius, 20, 20);

        // draw the bone
        glColor4f(0.65, 0.75, 1.0, 1.0);
        GLfloat bone_diffuse[] = {0.7, 1.0, 0.85, 1.0};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, bone_diffuse);
        drawTube(0, 0, 0, endX, endY, endZ, 0.75 * radius, 40, 40);

        glPopMatrix();
#else
	glCallList( _dplist[bone->_id] );
#endif

        glTranslatef(endX, endY, endZ);
    }

    for(int i=0;i<bone->_numChildren;i++)
    {
        BoneSegment* child = _m->_boneSegments[bone->_children[i]];
        drawBone( child );
    }

    glPopMatrix();
    glPopMatrix();
}

void HumanBodyModelRenderer::generateDisplayLists()
{
    if( _m )
        _dplistSize = _m->boneCount() + 1;
    else
	return;

    if( !_dplist )
	_dplist = new GLuint[_dplistSize];

    else
	glDeleteLists(_dplist[0], _dplistSize);

    _dplist[0] = glGenLists(_dplistSize);
    for(int i=0;i<_dplistSize;i++)
    {
	_dplist[i] = _dplist[0] + i;
	BoneSegment* bone = _m->_boneSegments[i];
        double endX, endY, endZ;
        endX = bone->_length * bone->_dir.x();
        endY = bone->_length * bone->_dir.y();
        endZ = bone->_length * bone->_dir.z();

        glNewList(_dplist[i], GL_COMPILE);

        glPushMatrix();
        // draw the joint
        glColor4f(1, 0, 0, 1);
        GLfloat joint_diffuse[] = {1, 0.75, 0.65, 1.0};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, joint_diffuse);
        double radius;
        if(bone->_length >= 1.0)
            radius = sqrt(bone->_length);
        else
            radius = bone->_length;
        glPushMatrix();
        glTranslatef(endX, endY, endZ);
        glutSolidSphere(0.5 * radius, 20, 20);
        glPopMatrix();

        // draw the bone
        glColor4f(0.65, 0.75, 1.0, 1.0);
        GLfloat bone_diffuse[] = {0.7, 1.0, 0.85, 1.0};
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, bone_diffuse);
        drawTube(0, 0, 0, endX, endY, endZ, 0.25 * radius, 2, 16);
	glPopMatrix();
	glEndList();
    }
}

void HumanBodyModelRenderer::drawAxis()
{
    glBegin(GL_LINES);
    // draw x axis in red, y axis in green, z axis in blue
    glColor3f(1., .2, .2);
    glVertex3f(0., 0., 0.);
    glVertex3f(.5, 0., 0.);

    glColor3f(.2, 1., .2);
    glVertex3f(0., 0., 0.);
    glVertex3f(0., .5, 0.);

    glColor3f(.2, .2, 1.);
    glVertex3f(0., 0., 0.);
    glVertex3f(0., 0., .5);

    glEnd();
}

void HumanBodyModelRenderer::drawTube(double x0, double y0, double z0,
                                      double x1, double y1, double z1,
                                      double radius, int layer, int slice)
{
    double layerStep = 1.0 / layer;
    double sliceStep = 1.0 / slice;
    DblPoint3D p0(x0, y0, z0), p1(x1, y1, z1);
    DblVector3D dirVec = p1 - p0;

    DblPoint3D infity(1e9, 1e9, 1e9);
    DblVector3D refVec = infity - p0;

    DblVector3D normVec1 = crossProduct(refVec, dirVec);
    normVec1 = normalize(normVec1);

    DblVector3D normVec2 = crossProduct(normVec1, dirVec);
    normVec2 = normalize(normVec2);

    DblVector3D rVec1 = radius * normVec1;
    DblVector3D rVec2 = radius * normVec2;

    for(int i=0;i<layer;i++)
    {
        double lRatio = layerStep * i;
        double nlRatio = lRatio + layerStep;

        DblPoint3D lPoint = p0 + lRatio * dirVec;
        DblPoint3D nlPoint = p0 + nlRatio * dirVec;

        for(int j=0;j<slice;j++)
        {
            double theta1 = (double) j * sliceStep * 2.0 * PI;
            double theta2 = (double) ( (j + 1) % slice ) * sliceStep * 2.0 * PI;

            DblVector3D disp1 = cos(theta1) * rVec1 + sin(theta1) * rVec2;
            DblVector3D disp2 = cos(theta2) * rVec1 + sin(theta2) * rVec2;

            DblPoint3D tp[4];
            DblPoint3D norm[4];
            tp[0] = lPoint + disp1;
            norm[0] = normalize(disp1);
            tp[1] = lPoint + disp2;
            norm[1] = normalize(disp2);
            tp[2] = nlPoint + disp2;
            norm[2] = normalize(disp2);
            tp[3] = nlPoint + disp1;
            norm[3] = normalize(disp1);

            glBegin(GL_QUADS);            
            for(int k=0;k<4;k++)
            {
                glNormal3f(norm[k].x(), norm[k].y(), norm[k].z());
                glVertex3f(tp[k].x(), tp[k].y(), tp[k].z());
            }
            glEnd();
        }
    }
}
