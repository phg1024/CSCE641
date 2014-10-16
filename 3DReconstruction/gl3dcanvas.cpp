#include "gl3dcanvas.h"

GL3DCanvas::GL3DCanvas(QWidget* parent, ProjectionMode mode):
        GLCanvasWindow(parent, qglformat_3d),
        projectionMode(mode),
        mouseState(UP),
        mouseInteractionMode(INTERACTION)
{
}

void GL3DCanvas::initializeGL()
{
    GLCanvasWindow::initializeGL();
    trackBall.init();
}

void GL3DCanvas::resizeGL(int w, int h)
{
    trackBall.reshape(w, h);

    glViewport (0, 0, w, h);

    glMatrixMode (GL_PROJECTION);

    glLoadIdentity ();

    switch(projectionMode)
    {
    case ORTHONGONAL:
        {
            glOrtho (-1.0, 1.0, -1.0, 1.0, 1.0, -1.0);
            break;
        }
    case FRUSTUM:
        {
            glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 1.0);
            break;
        }
    case PERSPECTIVE:
        {
            gluPerspective(60.0, (float)width()/(float)height(), 0.1f, 100.0f);
            break;
        }
    default:
        break;
    }

    glMatrixMode (GL_MODELVIEW);

    glLoadIdentity ();

    if(projectionMode==PERSPECTIVE)
    {
        gluLookAt(
                0.0, 0.0, 4.5,
                0.0, 0.0, 0.0,
                0.0, 1.0, 0.0);
    }

}

void GL3DCanvas::paintGL()
{
    GLCanvasWindow::paintGL();

    resizeGL(width(), height());
    glLoadIdentity();

    resizeGL(width(), height());
    //trackBall.printMatrix();
    glMatrixMode(GL_MODELVIEW);
    trackBall.applyTransform();
}

QPointF GL3DCanvas::transformMousePositionFromInput(const QPointF & pos) const
{
    QPointF p;
    p.setX(pos.x() / (float)width());
    p.setY(1.0 - pos.y() / (float)height());
    return p;
}

void GL3DCanvas::wheelEvent(QWheelEvent *e)
{
    MouseState preState = mouseState;
    mouseState = WHEEL;
    switch(mouseInteractionMode)
    {
    case VIEW_TRANSFORM:
        {
            trackBall.wheel(e->delta());
            updateGL();
            break;
        }
    case INTERACTION:
        break;
    default:
        break;
    }
    mouseState = preState;
}

void GL3DCanvas::mousePressEvent(QMouseEvent *e)
{
    mouseState = DOWN;
    switch(mouseInteractionMode)
    {
    case VIEW_TRANSFORM:
        {
            switch(e->buttons() & 0xF)
            {
            case Qt::MidButton:
                {
                    trackBall.mouse_translate(e->x(),e->y());
                    break;
                }
            case Qt::LeftButton:
                {
                    trackBall.mouse_rotate(e->x(), e->y());
                    break;
                }
            default:
                break;
            }            
            break;
        }
    case INTERACTION:
        break;
    }
    update();
}

void GL3DCanvas::mouseReleaseEvent(QMouseEvent *e)
{
    mouseState = UP;
}

void GL3DCanvas::mouseMoveEvent(QMouseEvent *e)
{
    switch(mouseInteractionMode)
    {
    case VIEW_TRANSFORM:
        {
            switch(e->buttons() & 0xF)
            {
            case Qt::MidButton:
                {
                    trackBall.motion_translate(e->x(),e->y());
                    update();
                    break;
                }
            case Qt::LeftButton:
                {
                    trackBall.motion_rotate(e->x(), e->y());
                    update();
                    break;
                }
            default:
                break;
            }
            break;
        }
    case INTERACTION:
        break;
    }    
}
