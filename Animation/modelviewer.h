#ifndef MODELVIEWER_H
#define MODELVIEWER_H

#include "GL/glew.h"
#include "GL/glut.h"

#include "gl3dcanvas.h"
#include "humanbodymodel.h"
#include "humanbodymodelrenderer.h"
#include "forwardkinematicsoperator.h"
#include "inversekinematicsoperator.h"

#include <QMovie>
#include <QThread>
#include <QTimer>
#include <cfloat>
using namespace std;

#define USE_ANIMATOR_THREAD 1

class AnimationThread;

class ModelViewer : public GL3DCanvas
{
    Q_OBJECT
public:
    ModelViewer(QWidget* parent = 0, const QGLFormat& format = qglformat_3d);
    ~ModelViewer();

    void bindModel(HumanBodyModel* m)
    {
        _model = m;
	_renderer.bindModel(_model);
        _fkop.bindModel( _model );
    }

    void releaseModel()
    {
        _model = 0;
    }

    void paint() { paintGL(); }

public slots:
    void switchInteractionState();
    void setCurrentFrame(int idx);
    void setAutoplaySpeed(double speed);
    void slot_switchReplay();
    void clearTrace();
    void setTraceBone(const QString&);
    void setTraceBonePosRatio(double);
    void toggleShowTrace();

protected:

    void paintGL();
    void initializeGL();
    void resizeGL(int w, int h);

    void keyPressEvent(QKeyEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *);

    void closeEvent(QCloseEvent *);

private:
    void storeCurrentMatrices();
    DblPoint3D project(const DblPoint3D& p);
    DblPoint3D backproject(const DblPoint3D &p);

    double _glproj[16];
    double _glmodelview[16];
    int _glviewport[4];

private:
    void makeGridTexture();
    void render();
    void renderGround();
    void renderTrace();
    void renderJoints();
    void highlightSelectedJoint();
    void renderTargetPoint();

    void enableLighting();
    void disableLighting();

private:
    void toggleAutoplay();
    void moveToNextFrame();
    void moveToPreviousFrame();

private:
    HumanBodyModel* _model;
    HumanBodyModelRenderer _renderer;
    int _currentFrame;
    bool _autoPlay;
    double _autoPlaySpeed;

    friend class AnimationThread;
    AnimationThread* _animator;

    ForwardKinematicsOperator _fkop;

private slots:
    void animate();

private:
    GLuint _gridTex;
    bool _isRecording;
    bool _showGround;

private:
    string _trace_joint_name;
    double _trace_joint_pos;
    vector<DblPoint3D> _trace;
    bool _showTrace;

private:
    bool _showAllJoints;
    bool _showConstraintPlane;


    // interaction related
private:
    vector<QPointF> _mouseTrace;
    QPointF _currentMousePos;

private:
    int _selectedJoint;
    bool _isManipulationMode;
    DblPoint3D _targetJointPos;

private:
    bool testBoneSelection(const QPointF&);
    void processRealtimeManipulation();
    double* getInitialConfiguration(int frameIdx = -1);
    double* getInitialConfiguration(const PostureParameters&);

private:
    PostureParameters _currentPosture;
    bool _showIKGeneratedPosture;
    PostureParameters _ikGeneratedPosture;
};

class AnimationThread : public QThread
{
    Q_OBJECT
public:
    AnimationThread():
        _doRendering(false)
    {
    }

    ~AnimationThread()
    {
    }

    void setFrameRate(double rate)
    {
        _frameRate = rate;
    }

    void setSpeed(double speed)
    {
        _speed = speed;
        cout << "current replay speed = " << speed << "X." << endl;
    }

    void setRenderingState(bool state)
    {
        _doRendering = state;
    }

    void stop() { _doRendering = false; }

    void run()
    {
        _doRendering = true;
        while( _doRendering )
        {
            emit sig_update();
            msleep( 1000.0 / _frameRate / _speed );
        }
    }

signals:
    void sig_update();

private:
    volatile bool _doRendering;
    double _frameRate;
    double _speed;
};

#endif // MODELVIEWER_H
