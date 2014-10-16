#include "GL/glew.h"
#include "GL/glut.h"

#include "modelviewer.h"
#include "scene.h"
#include "shape.h"

#include "imageviewer.h"

ModelViewer::ModelViewer(QWidget *parent) :    
    GL3DCanvas(parent),
    _scene(0),
    _vmode(OpenGL)
{
    connect(&_tracer, SIGNAL(sig_progress(double)), this, SIGNAL(sig_progress(double)));
}

void ModelViewer::bindScene(Scene *s)
{
    _scene = s;

    updateSceneParameters(width(), height());
    updateGL();
}

void ModelViewer::initializeGL()
{
    GL3DCanvas::initializeGL();
    sceneScale = 1.0;
    trackBall.setSceneScale(sceneScale);
}

void ModelViewer::resizeGL(int w, int h)
{
    if( _scene )
    {
        updateSceneParameters(w, h);
    }
    else
    {
        GL3DCanvas::resizeGL(w, h);
    }
}

void ModelViewer::paintGL()
{
    GL3DCanvas::paintGL();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SMOOTH);

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderScene();

    renderInfo();
}

void ModelViewer::updateSceneParameters(int w, int h)
{
    trackBall.reshape(w, h);

    glViewport (0, 0, w, h);

    glMatrixMode (GL_PROJECTION);

    glLoadIdentity ();

    // set parameters with given scene info
    const CameraInfo& camInfo = _scene->cameraInfo();

    const double PI = 3.1415296;
    double fovy = atan( 0.5 * camInfo._canvasSize[1] / camInfo._focalLength ) * 2.0;
    fovy = fovy / PI * 180.0;
    double aspectRatio = camInfo._canvasSize[0] / camInfo._canvasSize[1];
    double near = camInfo._focalLength;
    double far = 100.0 * camInfo._focalLength;

    gluPerspective(fovy, aspectRatio, near, far);

    glMatrixMode (GL_MODELVIEW);

    glLoadIdentity ();

    DblPoint3D viewingTarget = camInfo._pos;
    viewingTarget = viewingTarget + camInfo._dir;
    gluLookAt(  camInfo._pos.x(), camInfo._pos.y(), camInfo._pos.z(),
                viewingTarget.x(), viewingTarget.y(), viewingTarget.z(),
                camInfo._up.x(), camInfo._up.y(), camInfo._up.z() );
}

void ModelViewer::renderInfo()
{
    QString str;
    QTextStream sstr(&str);
    sstr << "Canvas size: " << width() << "x" << height();
    glColor4f(0, 0, 0, 1);
    renderText(20, 20, str);
}

void ModelViewer::renderScene()
{
    if( _scene )
    {
        switch( _vmode )
        {
        case OpenGL:
        {
            renderScene_OpenGL();
            break;
        }
        case RayTracing:
        {
            // render to a texture, then map it to the screen
            renderScene_RayTracing();
            break;
        }
        default:
        {
            break;
        }
        }
    }
}

void ModelViewer::renderScene_OpenGL()
{
    enableLighting();

//    if( _scene->hasGround() )
//        renderGround();

//    if( _scene->hasBoundingBox() )
//        renderBoundingBox();

    int shapeCount = _scene->shapeNumber();
    for(int i=0;i<shapeCount;i++)
    {
        const Shape* s = _scene->shape(i);

        switch( s->type() )
        {
        case Shape::SPHERE:
        {
            const Sphere* shape = dynamic_cast<const Sphere*>(s);
            DblPoint3D center = shape->center();
            DblColor4 color = shape->color();

            glColor4f(color.r(), color.g(), color.b(), color.a());
            GLfloat mat_diffuse[] = {color.r(), color.g(), color.b(), color.a()};
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);

            glPushMatrix();
            glTranslatef(center.x(), center.y(), center.z());
            glutSolidSphere(shape->radius(), 20, 20);
            glPopMatrix();
            break;
        }
        case Shape::TRIANGLE:
        {
            break;
        }
        case Shape::RECTANGLE:
        {
            const Rectangle* shape = dynamic_cast<const Rectangle*>(s);
            DblColor4 color = shape->color();

            glColor4f(color.r(), color.g(), color.b(), color.a());
            GLfloat mat_diffuse[] = {color.r(), color.g(), color.b(), color.a()};
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);

            glPushMatrix();

            glBegin(GL_QUADS);
            glNormal3f(shape->normal().x(), shape->normal().y(), shape->normal().z());
            for(int i=0;i<4;i++)
                glVertex3f(shape->vertex(i).x(), shape->vertex(i).y(), shape->vertex(i).z());
            glEnd();

            glPopMatrix();
            break;
        }
        case Shape::CUBE:
        {
            break;
        }
        case Shape::POLYGONMESH:
        {
            break;
        }
        default:
            break;
        }
    }

    disableLighting();
}

void ModelViewer::renderBoundingBox()
{
    // bounding box is set of a grey surfaces
    glPushMatrix();
    glColor4f(0.5, 0.5, 0.5, 1);
    GLfloat mat_diffuse[] = {0.5, 0.5, 0.5, 1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);

    // bottom
    glNormal3f(0, 1, 0);
    glBegin(GL_QUADS);
    glVertex3f( _scene->min(0), _scene->min(1), _scene->min(2));
    glVertex3f( _scene->min(0), _scene->min(1), _scene->max(2));
    glVertex3f( _scene->max(0), _scene->min(1), _scene->max(2));
    glVertex3f( _scene->max(0), _scene->min(1), _scene->min(2));
    glEnd();

    // top
    glNormal3f(0, -1, 0);
    glBegin(GL_QUADS);
    glVertex3f( _scene->min(0), _scene->max(1), _scene->min(2));
    glVertex3f( _scene->min(0), _scene->max(1), _scene->max(2));
    glVertex3f( _scene->max(0), _scene->max(1), _scene->max(2));
    glVertex3f( _scene->max(0), _scene->max(1), _scene->min(2));
    glEnd();

    // left
    glNormal3f(1, 0, 0);
    glBegin(GL_QUADS);
    glVertex3f( _scene->min(0), _scene->max(1), _scene->min(2));
    glVertex3f( _scene->min(0), _scene->max(1), _scene->max(2));
    glVertex3f( _scene->min(0), _scene->min(1), _scene->max(2));
    glVertex3f( _scene->min(0), _scene->min(1), _scene->min(2));
    glEnd();

    // right
    glNormal3f(-1, 0, 0);
    glBegin(GL_QUADS);
    glVertex3f( _scene->max(0), _scene->max(1), _scene->min(2));
    glVertex3f( _scene->max(0), _scene->max(1), _scene->max(2));
    glVertex3f( _scene->max(0), _scene->min(1), _scene->max(2));
    glVertex3f( _scene->max(0), _scene->min(1), _scene->min(2));
    glEnd();

    // back
    glNormal3f(0, 0, 1);
    glBegin(GL_QUADS);
    glVertex3f( _scene->max(0), _scene->max(1), _scene->min(2));
    glVertex3f( _scene->min(0), _scene->max(1), _scene->min(2));
    glVertex3f( _scene->min(0), _scene->min(1), _scene->min(2));
    glVertex3f( _scene->max(0), _scene->min(1), _scene->min(2));
    glEnd();

    glPopMatrix();
}

void ModelViewer::renderGround()
{
    // ground is a grey surface at y = 0
    glPushMatrix();
    glNormal3f(0, 1, 0);
    glColor4f(0.5, 0.5, 0.5, 1);
    GLfloat mat_diffuse[] = {0.5, 0.5, 0.5, 1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);

    glBegin(GL_QUADS);
    glVertex3f( _scene->min(0), 0, _scene->min(2));
    glVertex3f( _scene->min(0), 0, _scene->max(2));
    glVertex3f( _scene->max(0), 0, _scene->max(2));
    glVertex3f( _scene->max(0), 0, _scene->min(2));
    glEnd();
    glPopMatrix();
}

void ModelViewer::enableLighting()
{
    for(size_t i=0;i<_scene->lightSourcesNumber();i++)
    {
        LightSource l = _scene->lightSource(i);

        GLfloat light_position[] = {l._pos.x(),
                                    l._pos.y(),
                                    l._pos.z(),
                                    1.0};
        GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
        GLfloat mat_shininess[] = {50.0};
        GLfloat light_ambient[] = {0.05, 0.05, 0.05, 1.0};
        GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};

        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

        glLightfv(GL_LIGHT0 + i, GL_POSITION, light_position);
        glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, white_light);
        glLightfv(GL_LIGHT0 + i, GL_SPECULAR, white_light);
        glLightfv(GL_LIGHT0 + i, GL_AMBIENT, light_ambient);

        glEnable(GL_LIGHT0 + i);
    }
    glEnable(GL_LIGHTING);
}

void ModelViewer::disableLighting()
{
    for(size_t i=0;i<_scene->lightSourcesNumber();i++)
        glDisable(GL_LIGHT0 + i);
    glDisable(GL_LIGHTING);
}

void ModelViewer::renderScene_RayTracing()
{    

}

void ModelViewer::slot_render()
{
    _tracer.bindScene(_scene);
    _tracer.setSize(width(), height());
    _tracer.execute();
    // pop up a window showing the rendered image
    ImageViewer* _iv = new ImageViewer;
    RGBAImage img = _tracer.result();
    _iv->setImage(img.toQImage());
    _iv->show();
}

void ModelViewer::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_Space:
    {
        slot_render();
        e->accept();
        cout << "ray tracing complete." << endl;
    }
    default:
        break;
    }
}
