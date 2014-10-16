#ifndef MODELVIEWER_H
#define MODELVIEWER_H

#include "gl3dcanvas.h"
#include "raytracer.h"

class Scene;

class ModelViewer : public GL3DCanvas
{
    Q_OBJECT
public:
    enum ViewingMode{
        OpenGL,
        RayTracing
    };

    ModelViewer(QWidget *parent = 0);

    QSize sizeHint() const
    {
        return QSize(800, 600);
    }

    QSize minimumSizeHint() const
    {
        return QSize(640, 480);
    }

    void bindScene(Scene* s);
    void setViewingMode( ViewingMode m )
    {
        _vmode = m;
        updateGL();
    }

signals:
    void sig_progress(double);

public slots:
    void slot_render();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void updateSceneParameters(int w, int h);

protected:
    void keyPressEvent(QKeyEvent *e);

private:
    void renderScene();
    void renderScene_OpenGL();
    void renderScene_RayTracing();

    void renderInfo();
    void renderBoundingBox();
    void renderGround();

    void enableLighting();
    void disableLighting();

private:
    Scene* _scene;
    ViewingMode _vmode;
    RayTracer _tracer;
};

#endif // MODELVIEWER_H
