#include "modelviewer.h"
#include "voxelarraymodel.h"

const double ModelViewer::defaultScale = 2.0;

ModelViewer::ModelViewer(QWidget* parent):
    GL3DCanvas(parent),
    _model(0),
    _showWire(false),
    _lighting(false)
{
    mouseInteractionMode = VIEW_TRANSFORM;
}

ModelViewer::~ModelViewer()
{
}

void ModelViewer::initializeGL()
{
    glewInit();
    GL3DCanvas::initializeGL();

    // initialize shaders

    // generate textures

    // generate framebuffers
}

void ModelViewer::resizeGL(int w, int h)
{
    GL3DCanvas::resizeGL(w, h);
}

void ModelViewer::paintGL()
{
    GL3DCanvas::paintGL();
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_SMOOTH);

    if(_model != 0)
    {
        renderModel();
    }

    glDisable(GL_SMOOTH);
}

void ModelViewer::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_Space:
    {
        _lighting = !_lighting;
        updateGL();
        break;
    }
    case Qt::Key_W:
    {
        _showWire = !_showWire;
        updateGL();
        break;
    }
    default:
        break;
    }
}

void ModelViewer::keyReleaseEvent(QKeyEvent *e)
{
}

void ModelViewer::mousePressEvent(QMouseEvent *e)
{
    GL3DCanvas::mousePressEvent(e);
    if(mouseInteractionMode == INTERACTION)
    {

    }
    else
    {

    }
}

void ModelViewer::mouseReleaseEvent(QMouseEvent *e)
{
    GL3DCanvas::mouseReleaseEvent(e);
    if(mouseInteractionMode == INTERACTION)
    {

    }
    else
    {

    }
}

void ModelViewer::mouseDoubleClickEvent(QMouseEvent *e)
{
    GL3DCanvas::mouseDoubleClickEvent(e);
    if(mouseInteractionMode == INTERACTION)
    {

    }
}

void ModelViewer::mouseMoveEvent(QMouseEvent *e)
{
    GL3DCanvas::mouseMoveEvent(e);
    if(mouseInteractionMode == INTERACTION)
    {

    }
    else
    {

    }
}

void ModelViewer::wheelEvent(QWheelEvent *e)
{
    GL3DCanvas::wheelEvent(e);
    if(mouseInteractionMode == INTERACTION)
    {

    }
    else
    {

    }
}

void ModelViewer::renderTeapot()
{
    glColor4f(0.85, 0.95, 1.0, 1.0);

    GLfloat light_position[] = {1.5, 1.5, 1.5, 1.0};
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat mat_diffuse[] = {0.5, 0.5, 0.75, 1.0};
    GLfloat mat_shininess[] = {50.0};
    GLfloat light_ambient[] = {0.0, 0.0, 0.0, 1.0};
    GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
    //GLfloat lmodel_ambient[] = {0.25, 0.25, 0.25, 1.0};

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glutSolidTeapot(0.5);

    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
}

void ModelViewer::renderModel()
{
    if(_lighting)
    {
        enableLighting();
    }
    // naive rendering of volume model
    switch(_model->getType())
    {
    case AbstractModel::VOXELARRAY:
    {
        renderVoxelArray();
        break;
    }
    default:
        break;
    }

    if(_lighting)
        disableLighting();
}

void ModelViewer::setModel(AbstractModel *m)
{
    _model = m;

    updateGL();
}

void ModelViewer::renderVoxelArray()
{
    glEnable(GL_DEPTH_TEST);
    VoxelArrayModel* m = dynamic_cast<VoxelArrayModel*>(_model);
    if( m )
    {
        glPushMatrix();
        glScalef(defaultScale, defaultScale, defaultScale);
        renderUnitBoundingBox(m->getScaleX(), m->getScaleY(), m->getScaleZ());

        float scaleX, scaleY, scaleZ;
        scaleX = m->getScaleX(), scaleY = m->getScaleY(), scaleZ = m->getScaleZ();
        //        cout << scaleX << ", "
        //             << scaleY << ", "
        //             << scaleZ << endl;
        glPushMatrix();
        glScalef(scaleX, scaleY, scaleZ);
        const list<Voxel>& voxels = m->getVoxelArray();
        list<Voxel>::const_iterator vit = voxels.begin();
        while( vit != voxels.end() )
        {
            const Voxel& v = (*vit);

            glColor4f(v.r / 255.0, v.g / 255.0, v.b / 255.0, 1);

            //            cout << v.xMin << ", " << v.xMax << "\t"
            //            << v.yMin << ", " << v.yMax << "\t"
            //            << v.zMin << ", " << v.zMax << endl;
            renderCube(v.xMin, v.xMax, v.yMin, v.yMax, v.zMin, v.zMax);
            ++ vit;
        }
        glPopMatrix();
        glPopMatrix();
    }

    glDisable(GL_DEPTH_TEST);
}

void ModelViewer::renderCube(float x0, float x1,
                             float y0, float y1,
                             float z0, float z1)
{

    glBegin(GL_QUADS);

    // front
    glNormal3f(0, 0, -1);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y1, z0);
    glVertex3f(x1, y1, z0);
    glVertex3f(x1, y0, z0);

    // back
    glNormal3f(0, 0, 1);
    glVertex3f(x0, y0, z1);
    glVertex3f(x1, y0, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x0, y1, z1);

    // bottom
    glNormal3f(0, -1, 0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y0, z1);
    glVertex3f(x0, y0, z1);

    // top
    glNormal3f(0, 1, 0);
    glVertex3f(x0, y1, z0);
    glVertex3f(x0, y1, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y1, z0);

    // left
    glNormal3f(-1, 0, 0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y0, z1);
    glVertex3f(x0, y1, z1);
    glVertex3f(x0, y1, z0);

    // right
    glNormal3f(1, 0, 0);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y1, z0);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y0, z1);

    glEnd();

    if(_showWire)
    {
        glColor4f(0, 0, 0, 1);
        glBegin(GL_LINE_STRIP);
        // front
        glNormal3f(0, 0, -1);
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y1, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x1, y0, z0);
        glEnd();

        glBegin(GL_LINE_STRIP);
        // back
        glNormal3f(0, 0, 1);
        glVertex3f(x0, y0, z1);
        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x0, y1, z1);
        glEnd();

        glBegin(GL_LINE_STRIP);
        // bottom
        glNormal3f(0, -1, 0);
        glVertex3f(x0, y0, z0);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y0, z1);
        glVertex3f(x0, y0, z1);
        glEnd();

        glBegin(GL_LINE_STRIP);
        // top
        glNormal3f(0, 1, 0);
        glVertex3f(x0, y1, z0);
        glVertex3f(x0, y1, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y1, z0);
        glEnd();

        glBegin(GL_LINE_STRIP);
        // left
        glNormal3f(-1, 0, 0);
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y0, z1);
        glVertex3f(x0, y1, z1);
        glVertex3f(x0, y1, z0);
        glEnd();

        glBegin(GL_LINE_STRIP);
        // right
        glNormal3f(1, 0, 0);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y0, z1);

        glEnd();
    }
}

void ModelViewer::renderUnitBoundingBox(float xScale, float yScale, float zScale)
{
    float x0 = -0.5, x1 = 0.5,
            y0 = -0.5, y1 = 0.5,
            z0 = -0.5, z1 = 0.5;
    float x0_s = x0 * xScale, x1_s = x1 * xScale;
    float y0_s = y0 * yScale, y1_s = y1 * yScale;
    float z0_s = z0 * zScale, z1_s = z1 * zScale;
    glColor4f(0, 0, 0, 1);
    glBegin(GL_LINES);

    glColor3f(x0 + 0.5, y0 + 0.5, z0 + 0.5);glVertex3f(x0_s, y0_s, z0_s);
    glColor3f(x1 + 0.5, y0 + 0.5, z0 + 0.5);glVertex3f(x1_s, y0_s, z0_s);

    glColor3f(x0 + 0.5, y1 + 0.5, z0 + 0.5);glVertex3f(x0_s, y1_s, z0_s);
    glColor3f(x1 + 0.5, y1 + 0.5, z0 + 0.5);glVertex3f(x1_s, y1_s, z0_s);

    glColor3f(x0 + 0.5, y1 + 0.5, z1 + 0.5);glVertex3f(x0_s, y1_s, z1_s);
    glColor3f(x1 + 0.5, y1 + 0.5, z1 + 0.5);glVertex3f(x1_s, y1_s, z1_s);

    glColor3f(x0 + 0.5, y0 + 0.5, z1 + 0.5);glVertex3f(x0_s, y0_s, z1_s);
    glColor3f(x1 + 0.5, y0 + 0.5, z1 + 0.5);glVertex3f(x1_s, y0_s, z1_s);

    glColor3f(x0 + 0.5, y0 + 0.5, z1 + 0.5);glVertex3f(x0_s, y0_s, z1_s);
    glColor3f(x0 + 0.5, y1 + 0.5, z1 + 0.5);glVertex3f(x0_s, y1_s, z1_s);

    glColor3f(x0 + 0.5, y0 + 0.5, z0 + 0.5);glVertex3f(x0_s, y0_s, z0_s);
    glColor3f(x0 + 0.5, y1 + 0.5, z0 + 0.5);glVertex3f(x0_s, y1_s, z0_s);

    glColor3f(x1 + 0.5, y0 + 0.5, z1 + 0.5);glVertex3f(x1_s, y0_s, z1_s);
    glColor3f(x1 + 0.5, y1 + 0.5, z1 + 0.5);glVertex3f(x1_s, y1_s, z1_s);

    glColor3f(x1 + 0.5, y0 + 0.5, z0 + 0.5);glVertex3f(x1_s, y0_s, z0_s);
    glColor3f(x1 + 0.5, y1 + 0.5, z0 + 0.5);glVertex3f(x1_s, y1_s, z0_s);

    glColor3f(x0 + 0.5, y0 + 0.5, z0 + 0.5);glVertex3f(x0_s, y0_s, z0_s);
    glColor3f(x0 + 0.5, y0 + 0.5, z1 + 0.5);glVertex3f(x0_s, y0_s, z1_s);

    glColor3f(x0 + 0.5, y1 + 0.5, z0 + 0.5);glVertex3f(x0_s, y1_s, z0_s);
    glColor3f(x0 + 0.5, y1 + 0.5, z1 + 0.5);glVertex3f(x0_s, y1_s, z1_s);

    glColor3f(x1 + 0.5, y0 + 0.5, z0 + 0.5);glVertex3f(x1_s, y0_s, z0_s);
    glColor3f(x1 + 0.5, y0 + 0.5, z1 + 0.5);glVertex3f(x1_s, y0_s, z1_s);

    glColor3f(x1 + 0.5, y1 + 0.5, z0 + 0.5);glVertex3f(x1_s, y1_s, z0_s);
    glColor3f(x1 + 0.5, y1 + 0.5, z1 + 0.5);glVertex3f(x1_s, y1_s, z1_s);

    glEnd();
}

void ModelViewer::enableLighting()
{
    GLfloat light_position[] = {2.5, -2.5, 2.5, 1.0};
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat mat_diffuse[] = {0.85, 0.95, 0.85, 1.0};
    GLfloat mat_shininess[] = {25.0};
    GLfloat light_ambient[] = {0.05, 0.05, 0.05, 1.0};
    GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
    //GLfloat lmodel_ambient[] = {0.25, 0.25, 0.25, 1.0};

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

    light_position[0] = -2.5, light_position[1] = -2.5, light_position[2] = -2.5;
    glLightfv(GL_LIGHT1, GL_POSITION, light_position);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT1, GL_SPECULAR, white_light);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
}

void ModelViewer::disableLighting()
{
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
}

