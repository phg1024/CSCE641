#ifndef MODELVIEWER_H
#define MODELVIEWER_H

#include "GL/glew.h"
#include "GL/glut.h"

#include "gl3dcanvas.h"
#include "abstractmodel.h"

class ModelViewer : public GL3DCanvas
{
public:
    ModelViewer(QWidget* parent = 0);
    ~ModelViewer();

    QSize sizeHint() const {return QSize(640, 480);}

    void setModel(AbstractModel* m);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);


private:
    void initializeShaders();
    void initializeFramebuffers();

private:
    void renderTeapot();
    void renderModel();
    void renderVoxelArray();
    void renderCube(float, float, float, float, float, float);
    void renderUnitBoundingBox(float xScale = 1.0, float yScale = 1.0, float zScale = 1.0);
    void pointBasedVolumeRendering();

private:
    AbstractModel* _model;

private:
    static const double defaultScale;
    bool _showWire;
    bool _lighting;

    void enableLighting();
    void disableLighting();
};

#endif // MODELVIEWER_H
