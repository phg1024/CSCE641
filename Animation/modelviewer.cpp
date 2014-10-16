#include "GL/glew.h"
#include "GL/glut.h"
#include "modelviewer.h"
#include "humanbodymodel.h"

ModelViewer::ModelViewer(QWidget *parent, const QGLFormat& format):
    GL3DCanvas(parent, format),
    _model(0),
    _currentFrame(-1),
    _autoPlay(false),
    _autoPlaySpeed(1.0),    
    _animator(0),
    _isRecording(false),
    _showGround(true),
    _trace_joint_name(""),
    _trace_joint_pos(1.0),
    _showTrace(false),
    _showAllJoints(false),
    _showConstraintPlane(false),
    _selectedJoint(-1),
    _isManipulationMode(false),
    _showIKGeneratedPosture(false)
{
    mouseInteractionMode = VIEW_TRANSFORM;
    _animator = new AnimationThread();
    connect(_animator, SIGNAL(sig_update()), this, SLOT(animate()));
}

ModelViewer::~ModelViewer()
{
    _animator->stop();
    _animator->exit();
    delete _animator;
}

void ModelViewer::initializeGL()
{
    GL3DCanvas::initializeGL();
    glewInit();

    glEnable(GL_MULTISAMPLE_ARB);

    QGLFormat fmt = format();
    if( fmt.sampleBuffers() )
    {
	cout << "using multisample buffers." << endl;
	cout << "samples = " << fmt.samples() << endl;
    }

    GLint bufs;
    GLint samples;
    glGetIntegerv(GL_SAMPLE_BUFFERS, &bufs);
    glGetIntegerv(GL_SAMPLES, &samples);
    cout << "using " << bufs << " buffers and " << samples << " samples." << endl;

    makeGridTexture();
}

void ModelViewer::resizeGL(int w, int h)
{
    GL3DCanvas::resizeGL(w, h);
}

void ModelViewer::paintGL()
{
    render();

    if(_isRecording)
    {
        if( _currentFrame >= 0 )
        {
            QImage screenshot = this->grabFrameBuffer(true);
            QString fname;
            QTextStream s(&fname);
            int digits = ceil(log10(_currentFrame + 0.5));
            if( digits == 0 )
                digits ++;
            const int totalDigits = 4;
            for(int i=0;i<totalDigits - digits;i++)
                s << '0';
            s << _currentFrame << ".png";
            screenshot.save(fname);
        }
    }
}

void ModelViewer::makeGridTexture()
{
    const int size = 64;
    unsigned char* gridData = new unsigned char[size * size * 3];
    for(int i=0;i<size;i++)
        for(int j=0;j<size;j++)
        {
            unsigned char c = ((((i&0x8)==0)^(((j&0x8))==0)))*255;
            int offset = (i * size + j) * 3;
            gridData[offset + 0] = c * 0.65;
            gridData[offset + 1] = c * 0.65;
            gridData[offset + 2] = c * 0.65;
        }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures( 1, &_gridTex );
    glBindTexture( GL_TEXTURE_2D, _gridTex );
    glEnable(GL_TEXTURE_2D);

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                     GL_NEAREST_MIPMAP_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, size, size, GL_RGB, GL_UNSIGNED_BYTE, gridData);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, gridData);

    glDisable(GL_TEXTURE_2D);

    delete[] gridData;
}

void ModelViewer::render()
{
    GL3DCanvas::paintGL();

    storeCurrentMatrices();

    glClearColor(0, 0, 0, 0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glClearColor(0.35, 0.35, 0.35, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_SMOOTH);
    glEnable(GL_POINT_SMOOTH);

    enableLighting();

    if(_showGround)
    	renderGround();

    glPushMatrix();
    // render the model
    if( _model )
    {
        if( _isManipulationMode )
        {
            _renderer.draw(_currentPosture, DblVector3D(0, 0, 0));
        }
        else
        {
            _renderer.draw(_currentFrame);
        }
    }
    glPopMatrix();

    glPushMatrix();
    if( _model && _showIKGeneratedPosture )
    {
        // for testing
        int initConfFrame = _currentFrame - 25;
        if( initConfFrame < 0 ) initConfFrame = 0;

        double* initConf = getInitialConfiguration(initConfFrame);
        if( initConf )
        {
            vector<DblPoint3D> joints = _fkop.getAllJointPositions(_currentPosture, 1);

            InverseKinematicsOperator::initializeOperator(_model);
            InverseKinematicsOperator::setUseJointParamConstraints(false);
            InverseKinematicsOperator::setUseRandomInitialConfigurations(true);

            PostureParameters _ikGeneratedPosture = InverseKinematicsOperator::getPosture(joints, 1, initConf);

            delete[] initConf;
            //_currentPosture.print("current posture:");
            //_ikGeneratedPosture.print("generated posture:");
            _renderer.draw(_ikGeneratedPosture, DblVector3D(20, 0, 0));
        }
    }
    glPopMatrix();

    if( _showTrace )
        renderTrace();

    if( _isManipulationMode )
        renderTargetPoint();

    if( _selectedJoint != -1 )
        highlightSelectedJoint();

    if( _showAllJoints )
        renderJoints();

    disableLighting();

    if( _model )
    {
        glColor4f(1, 1, 1, 1);
        int lineIdx = 1;
        QString info;
        QTextStream infostr(&info);
        infostr << "Current frame: " << _currentFrame << " / " << _model->frameNumber() - 1;
        renderText(20, 20 * lineIdx++, info);

        info.clear();
        infostr << "Replay speed: " << _autoPlaySpeed;
        renderText(20, 20 * lineIdx++, info);

        if( _isManipulationMode )
        {
            info.clear();
            infostr << "Direct Manipulation";
            renderText(20, 20 * lineIdx++, info);
        }
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_SMOOTH);
    glDisable(GL_MULTISAMPLE);
}

void ModelViewer::renderGround()
{
    const float sceneScale = 80.0;
    const float groundPos = 0.0;
    glColor4f(1, 1, 1, 1);
    GLfloat mat_diffuse[] = {0.95, 0.95, 0.95, 1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _gridTex);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glTexCoord2f(0.0, 0.0); glVertex3f(-sceneScale, groundPos, sceneScale);
    glTexCoord2f(0.0, 1.0); glVertex3f(-sceneScale, groundPos, -sceneScale);
    glTexCoord2f(1.0, 1.0); glVertex3f(sceneScale, groundPos, -sceneScale);
    glTexCoord2f(1.0, 0.0); glVertex3f(sceneScale, groundPos, sceneScale);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void ModelViewer::renderTargetPoint()
{
    if( _selectedJoint < 0 )
        return;

    GLfloat mat_diffuse[] = {0.85, 0.85, 0.85, 1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glPushMatrix();
    glColor3f(1, 1, 1);
    glTranslatef(_targetJointPos.x(), _targetJointPos.y(), _targetJointPos.z());
    glutSolidSphere(0.75, 20, 20);
    glPopMatrix();
}

void ModelViewer::renderTrace()
{
    if( !_model )
        return;

    if(_trace.empty())
        return;

    GLfloat mat_diffuse[] = {0.95, 0.95, 0.35, 1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);

    glPushMatrix();
    float s = _model->scale();
    glScalef(s, s, s);
    glColor4f(1, 1, 1, 1);
#if 1
    // use large offset to get smoother gradients
    const int gradientOffset = 5;
    glLineWidth(2.0);
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<_currentFrame;i++)
    {
        int prevIdx = (i - gradientOffset);
        if(prevIdx < 0) prevIdx += _currentFrame;
        DblPoint3D& prev = _trace[prevIdx];
        int nextIdx = (i + gradientOffset);
        if(nextIdx > _currentFrame - 1) prevIdx -= _currentFrame;
        DblPoint3D& next = _trace[nextIdx];

        DblPoint3D& p = _trace[i];

        DblVector3D norm = 0.5 * (prev + next - 2.0 * p);
        norm = normalize(norm);

        glNormal3f(norm.x(), norm.y(), norm.z());
        glVertex3f(p.x(), p.y(), p.z());
    }
    glEnd();
    glLineWidth(1.0);
#else
    for(int i=0;i<_currentFrame - 1;i++)
    {
        DblPoint3D& p = _trace[i];
        DblPoint3D& next = _trace[i+1];

        HumanBodyModelRenderer::drawTube(p.x(), p.y(), p.z(),
                                         next.x(), next.y(), next.z(),
                                         0.1, 2, 8);
    }
#endif
    glPopMatrix();
}

void ModelViewer::renderJoints()
{
    if( !_model )
        return;

    vector<DblPoint3D> jointPos = _fkop.getAllJointPositions(_currentPosture, 1.0);

    GLfloat joint_diffuse[] = {0.85, 0.85, 0.85, 1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, joint_diffuse);
    int bIdx = 0;
    for(vector<DblPoint3D>::iterator it = jointPos.begin(); it != jointPos.end(); it++)
    {
    BoneSegment* bone = _model->_boneSegments[bIdx];
    double radius;
    if(bone->_length >= 1.0)
        radius = sqrt(bone->_length);
    else
        radius = bone->_length;
    const double epsilon = 5e-2;
//    cout << "#" << bIdx << ": " << radius << endl;
    glPushMatrix();
        glTranslatef((*it).x(), (*it).y(), (*it).z());
    	glutSolidSphere(0.5 * radius + epsilon, 20, 20);
    glPopMatrix();
    bIdx++;
    }
}

void ModelViewer::enableLighting()
{
    GLfloat light_position[] = {60.0,
                                60.0,
                                60.0,
                                1.0};
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    //GLfloat mat_diffuse[] = {0.85, 0.95, 0.85, 1.0};
    GLfloat mat_shininess[] = {25.0};
    GLfloat light_ambient[] = {0.05, 0.05, 0.05, 1.0};
    GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
    //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

    light_position[0]= -60;
    light_position[1] = 60;
    light_position[2] = -60;
    glLightfv(GL_LIGHT1, GL_POSITION, light_position);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT1, GL_SPECULAR, white_light);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
}


void ModelViewer::highlightSelectedJoint()
{
    if( !_model )
        return;

    DblPoint3D jointPos = _fkop.getGlobalPosition(_currentPosture, _selectedJoint, 1.0);

    BoneSegment* bone = _model->_boneSegments[_selectedJoint];
    GLfloat joint_diffuse[] = {0.85, 0.85, 0.35, 1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, joint_diffuse);
    double radius;
    if(bone->_length >= 1.0)
        radius = sqrt(bone->_length);
    else
        radius = bone->_length;
    const double epsilon = 5e-2;
    glPushMatrix();
    glTranslatef(jointPos.x(), jointPos.y(), jointPos.z());
    glutSolidSphere(0.5 * radius + epsilon, 20, 20);
    glPopMatrix();

    if( _showConstraintPlane )
    {
        // the constraint plane is the plane orthogonal to viewing
        // direction, which is used when manipulating the model:
        // lower cost will be assigned to postures in which the
        // selected joint is closer to the constraint plane.

        DblPoint3D screenJointPos = project(jointPos);
        DblPoint3D corners[4];
        double offset = 100;
        corners[0].x() = screenJointPos.x() - offset;
        corners[0].y() = screenJointPos.y() - offset;
        corners[0].z() = screenJointPos.z();
        corners[1].x() = screenJointPos.x() - offset;
        corners[1].y() = screenJointPos.y() + offset;
        corners[1].z() = screenJointPos.z();
        corners[2].x() = screenJointPos.x() + offset;
        corners[2].y() = screenJointPos.y() + offset;
        corners[2].z() = screenJointPos.z();
        corners[3].x() = screenJointPos.x() + offset;
        corners[3].y() = screenJointPos.y() - offset;
        corners[3].z() = screenJointPos.z();

        glPushMatrix();
        glBegin(GL_QUADS);
        for(int i=0;i<4;i++)
        {
            DblPoint3D p = backproject( corners[i] );
            cout << p << endl;
            glVertex3f(p.x(), p.y(), p.z());
        }
        glEnd();
        glPopMatrix();
    }
}

void ModelViewer::disableLighting()
{
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHTING);
}

void ModelViewer::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_PageDown:
    {
        moveToPreviousFrame();
        break;
    }
    case Qt::Key_PageUp:
    {
        moveToNextFrame();
        break;
    }
    case Qt::Key_Space:
    {
        _isRecording = !_isRecording;
        toggleAutoplay();
        break;
    }
    case Qt::Key_A:
    {
        _autoPlaySpeed *= 2.0;
        _animator->setSpeed(_autoPlaySpeed);
        break;
    }
    case Qt::Key_D:
    {
        _autoPlaySpeed /= 2.0;
        _animator->setSpeed(_autoPlaySpeed);
        break;
    }
    case Qt::Key_G:
    {
	_showGround = !_showGround;
	updateGL();
	break;
    }
    case Qt::Key_J:
    {
        _showAllJoints = !_showAllJoints;
        updateGL();
        break;
    }
    case Qt::Key_I:
    {
        _showIKGeneratedPosture = !_showIKGeneratedPosture;
        updateGL();
        break;
    }
    case Qt::Key_P:
    {
	switch( projectionMode )
	{
	case PERSPECTIVE:
	    {
    		projectionMode = ORTHONGONAL;
	    break;
	    }
	case ORTHONGONAL:
	    {
		projectionMode = PERSPECTIVE;
		break;
	    }
	default:
	    break;
	}
	updateGL();
	break;
    }
    case Qt::Key_Control:
    {
        _isManipulationMode = !_isManipulationMode;
        updateGL();
        break;
    }
    default:
        break;
    }
}

void ModelViewer::mousePressEvent(QMouseEvent *e)
{
    GL3DCanvas::mousePressEvent(e);

    switch(mouseInteractionMode)
    {
    case INTERACTION:
    {
        _mouseTrace.clear();

        QPointF mPos = transformMousePositionFromInput(e->posF());
        _mouseTrace.push_back(mPos);
        _currentMousePos = mPos;

        if( testBoneSelection( _currentMousePos ) )
        {
        }

        break;
    }
    default:
    {
        break;
    }
    }
}

void ModelViewer::mouseReleaseEvent(QMouseEvent *e)
{
    GL3DCanvas::mouseReleaseEvent(e);

    switch(mouseInteractionMode)
    {
    case INTERACTION:
    {
        QPointF mPos = transformMousePositionFromInput(e->posF());
        _mouseTrace.push_back(mPos);
        _currentMousePos = mPos;

        if( _isManipulationMode )
        {
            processRealtimeManipulation();
        }

        break;
    }
    default:
    {
        break;
    }
    }
}

void ModelViewer::mouseMoveEvent(QMouseEvent *e)
{
    GL3DCanvas::mouseMoveEvent(e);

    switch(mouseInteractionMode)
    {
    case INTERACTION:
    {
        switch( mouseState )
        {
        case DOWN:
        {
            QPointF mPos = transformMousePositionFromInput(e->posF());
            _mouseTrace.push_back(mPos);
            _currentMousePos = mPos;

            if( _isManipulationMode && _selectedJoint > 0 )
            {
                DblPoint3D jointPos = _fkop.getGlobalPosition(_currentPosture, _selectedJoint, 1.0);

                // project joint position to screen position
                DblPoint3D screenJointPos = project(jointPos);

                // back project mouse position on screen to 3D position
                int x = _currentMousePos.x() * width(), y = _currentMousePos.y() * height();
                double z = screenJointPos.z();
                // back projected 3D position
                DblPoint3D p = backproject( DblPoint3D(x, y, z) );

                _targetJointPos = p;
            }

            updateGL();
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
    {
        break;
    }
    }
}

void ModelViewer::mouseDoubleClickEvent(QMouseEvent *e)
{
    GL3DCanvas::mouseDoubleClickEvent(e);
}

void ModelViewer::closeEvent(QCloseEvent *e)
{    
    _animator->stop();
    _animator->exit();
    QGLWidget::closeEvent(e);
}

void ModelViewer::toggleAutoplay()
{
    _autoPlay = !_autoPlay;
    if(_autoPlay)
    {
#if USE_ANIMATOR_THREAD
        _animator->setFrameRate(120.0);
        _animator->setSpeed(_autoPlaySpeed);
        _animator->setRenderingState(true);
        _animator->start();
#else
        int maxFrame = _model->frameNumber();
        while(1)
        {
            qApp->processEvents();
            if( _currentFrame < maxFrame - 1 )
                _currentFrame ++;
            else
            {
                _currentFrame = 0;
                updateGL();
                break;
            }
            updateGL();
        }
#endif
    }
#if USE_ANIMATOR_THREAD
    else
    {
        _animator->stop();
    }
#endif
}

void ModelViewer::moveToPreviousFrame()
{
    if( _currentFrame > 0 )
    {
        _currentFrame --;
        _currentPosture = _model->_frames[_currentFrame];
        cout << "current frame = " << _currentFrame << endl;
    }
    updateGL();
}

void ModelViewer::moveToNextFrame()
{
    if(_model->frameNumber() > 0 && _currentFrame < _model->frameNumber() - 1 )
    {
        _currentFrame ++;
        _currentPosture = _model->_frames[_currentFrame];
        cout << "current frame = " << _currentFrame << endl;
    }
    updateGL();
}

void ModelViewer::switchInteractionState()
{
    if( mouseInteractionMode == GL3DCanvas::INTERACTION )
        mouseInteractionMode = GL3DCanvas::VIEW_TRANSFORM;
    else
        mouseInteractionMode = GL3DCanvas::INTERACTION;
}

void ModelViewer::slot_switchReplay()
{
    toggleAutoplay();
}

void ModelViewer::setCurrentFrame(int idx)
{    
    _currentFrame = idx;
    if( _model )
    {
        if( idx >= 0)
        {
            _currentPosture = _model->_frames.at(idx);
        }
    }
    updateGL();
}

void ModelViewer::setAutoplaySpeed(double speed)
{
    _autoPlaySpeed = speed;

    _animator->setSpeed( _autoPlaySpeed );
}

void ModelViewer::setTraceBone(const QString &name)
{
    _trace_joint_name = name.toStdString();

    if(_trace_joint_name == "")
        return;

    int boneIdx = _model->_boneIndexMap[_trace_joint_name];
    BoneSegment *bone = _model->_boneSegments[boneIdx];
    _trace = _fkop.getTrace(boneIdx, bone->_length * _trace_joint_pos);
}

void ModelViewer::setTraceBonePosRatio(double ratio)
{
    _trace_joint_pos = ratio;

    if(_trace_joint_name == "")
        return;

    int boneIdx = _model->_boneIndexMap[_trace_joint_name];
    BoneSegment *bone = _model->_boneSegments[boneIdx];
    _trace = _fkop.getTrace(boneIdx, bone->_length * _trace_joint_pos);
}

void ModelViewer::toggleShowTrace()
{
    _showTrace = !_showTrace;
    updateGL();
}

void ModelViewer::clearTrace()
{
    _trace.clear();
}

// interactions
bool ModelViewer::testBoneSelection(const QPointF &p)
{
    if( _model )
    {
        bool findMatched = false;

        // retrieve all joint positions
        vector<DblPoint3D> jointPositions = _fkop.getAllJointPositions(_currentPosture, 1.0);
        const double distanceThreshold = 0.05 * sqrt( width() * height() );

        // perform projection
        vector<DblPoint3D> projectedJointPositions;
        for(size_t i=0;i<jointPositions.size();i++)
        {
            const DblPoint3D& p = jointPositions[i];

            DblPoint3D projectedPos = project(p);

            projectedJointPositions.push_back(projectedPos);
        }

        DblPoint2D regularMousePosition(p.x() * width(), p.y() * height());
        double matchedPointDistance = DBL_MAX;
        int matchedIdx = -1;
        DblPoint3D matchedPos;
        // for all projected points within given distance threshold, pick the nearest one
        for(size_t i=0;i<projectedJointPositions.size();i++)
        {
            DblPoint3D candidate = projectedJointPositions[i];
            double dist = GeometryUtils::distance(candidate.xy(), regularMousePosition);
            if( dist <= distanceThreshold )
            {
                if( dist < matchedPointDistance )
                {
                    matchedPointDistance = dist;
                    matchedIdx = i;
                    matchedPos = candidate;
                }
            }
        }

        if( matchedIdx > 0 )
        {
            cout << "Matched joint: " << _model->_boneSegments[matchedIdx]->_name << " @ " << matchedPos << endl;
            _selectedJoint = matchedIdx;
            findMatched = true;
        }
        else
            cout << "No match found!" << endl;

        return findMatched;
    }
    else
        return false;
}

void ModelViewer::processRealtimeManipulation()
{
#if 0
    cout << "processing realtime manipulation ..." << endl;
    DblPoint3D jointPos = _fkop.getGlobalPosition(_currentPosture, _selectedJoint, 1.0);

    // project joint position to screen position
    DblPoint3D screenJointPos = project(jointPos);

    // back project mouse position on screen to 3D position
    int x = _currentMousePos.x() * width(), y = _currentMousePos.y() * height();
    double z = screenJointPos.z();
    // back projected 3D position
    DblPoint3D p = backproject( DblPoint3D(x, y, z) );

    // now use inverse kinematics to obtain desired configuration

    double* initConf = 0;
    initConf = getInitialConfiguration(_currentFrame);
    if( !initConf )
        return;

    vector<DblPoint3D> joints = _fkop.getAllJointPositions(_currentPosture, 1);
    // replace the selected joint with desired position
    joints[_selectedJoint] = p;

    InverseKinematicsOperator::initializeOperator(_model);
    InverseKinematicsOperator::setUseJointParamConstraints(true);
    InverseKinematicsOperator::setUseRandomInitialConfigurations(false);

    PostureParameters _ikGeneratedPosture = InverseKinematicsOperator::getPosture(joints, 1, initConf);

    delete[] initConf;
    //_currentPosture.print("current posture:");
    //_ikGeneratedPosture.print("generated posture:");
    _currentPosture = _ikGeneratedPosture;
    updateGL();
#else
    cout << "processing realtime manipulation ..." << endl;
    DblPoint3D jointPos = _fkop.getGlobalPosition(_currentPosture, _selectedJoint, 1.0);

    // project joint position to screen position
    DblPoint3D screenJointPos = project(jointPos);

    // back project mouse position on screen to 3D position
    int x = _currentMousePos.x() * width(), y = _currentMousePos.y() * height();
    double z = screenJointPos.z();
    // back projected 3D position
    DblPoint3D p = backproject( DblPoint3D(x, y, z) );

    _targetJointPos = p;

    // now use inverse kinematics to obtain desired configuration

    double* initConf = 0;
    initConf = getInitialConfiguration(_currentPosture);
    if( !initConf )
        return;

    InverseKinematicsOperator::initializeOperator(_model);
    InverseKinematicsOperator::setUseJointParamConstraints(true);
    InverseKinematicsOperator::setUseRandomInitialConfigurations(false);

    PostureParameters _ikGeneratedPosture = InverseKinematicsOperator::getPosture(p, _selectedJoint, 1, initConf);

    delete[] initConf;
    //_currentPosture.print("current posture:");
    //_ikGeneratedPosture.print("generated posture:");
    _currentPosture = _ikGeneratedPosture;
    updateGL();
#endif
}

double* ModelViewer::getInitialConfiguration(const PostureParameters& pose)
{
    int m = 6;  // root dof
    for(size_t i=1;i<=_model->getBoneCount();i++)
        m += _model->getBone(i)->_dof;

    double *initConf = new double[m];

    int initConfIdx = 0;

    for(int i=0;i<6;i++)
        initConf[initConfIdx++] = pose._boneParams[0]._params[i];
    for(size_t i=1;i<=_model->getBoneCount();i++)
    {
        for(int j=0;j<pose._boneParams[i]._numParams;j++)
            initConf[initConfIdx++] = pose._boneParams[i]._params[j];
    }

    return initConf;
}

double* ModelViewer::getInitialConfiguration(int initConfFrame)
{
    int m = 6;  // root dof
    for(size_t i=1;i<=_model->getBoneCount();i++)
        m += _model->getBone(i)->_dof;

    double *initConf = new double[m];

    if( initConfFrame < 0 )
    {
        // randomly assign value
        for(int i=0;i<m;i++)
            initConf[i] = ((double)rand() / (double)RAND_MAX - 0.5) * 180.0;
    }
    else
    {
        int initConfIdx = 0;
        PostureParameters& initPose = _model->_frames[initConfFrame];
        for(int i=0;i<6;i++)
            initConf[initConfIdx++] = initPose._boneParams[0]._params[i];
        for(size_t i=1;i<=_model->getBoneCount();i++)
        {
            for(int j=0;j<initPose._boneParams[i]._numParams;j++)
                initConf[initConfIdx++] = initPose._boneParams[i]._params[j];
        }
    }

    return initConf;
}

void ModelViewer::storeCurrentMatrices()
{
    glGetDoublev (GL_MODELVIEW_MATRIX, _glmodelview);
    glGetDoublev (GL_PROJECTION_MATRIX, _glproj);
    glGetIntegerv (GL_VIEWPORT, _glviewport);
}

DblPoint3D ModelViewer::project(const DblPoint3D &p)
{
    GLdouble orix;
    GLdouble oriy;
    GLdouble oriz;
    gluProject(p.x(), p.y(), p.z(),
               _glmodelview, _glproj, _glviewport,
               &orix, &oriy, &oriz
               );

    DblPoint3D projectedPos(orix, oriy, oriz);

    return projectedPos;
}

DblPoint3D ModelViewer::backproject(const DblPoint3D &p)
{
    GLdouble orix;
    GLdouble oriy;
    GLdouble oriz;
    gluUnProject(p.x(), p.y(), p.z(),
                 _glmodelview, _glproj, _glviewport,
                 &orix, &oriy, &oriz
                 );

    DblPoint3D projectedPos(orix, oriy, oriz);

    return projectedPos;
}

void ModelViewer::animate()
{
    int maxFrame = _model->frameNumber();

    if( _currentFrame < maxFrame - 1 )
    {
        _currentFrame++;
        _currentPosture = _model->getFrame(_currentFrame);
    }
    else
    {
        _currentFrame = 0;
    }
    update();
}



