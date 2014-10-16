#include "util_common.h"
#include "glutils.h"

using namespace Utils;

void GLUtils::drawGradient(const QPointF &pos, const QColor& c, const float &size, const float &aspectRatio, int layerNum, int segNum)
{
    glTranslatef(pos.x(), pos.y(), 0.0);
    glScalef(1.0 / aspectRatio, 1.0, 1.0);

    // calculate constants
    float *sinTheta = new float[segNum];
    float *cosTheta = new float[segNum];
    
    const float twoPI = 2.0 * PI;
    for(int i=0;i<segNum;i++)
    {
        float ratio = (float)i / (float)segNum;
        sinTheta[i] = sin(twoPI * ratio);
        cosTheta[i] = cos(twoPI * ratio);
    }

    for(int i=0;i<layerNum - 1;i++)
    {
        int innerLayer = i;
        int outerLayer = i + 1;
        float innerRatio = (float)innerLayer / (float)layerNum;
        float innerAlpha = 1.0 - innerRatio;
        float outerRatio = (float)outerLayer / (float)layerNum;
        float outerAlpha = 1.0 - outerRatio;
        float innerRadius = innerRatio * size;
        float outerRadius = outerRatio * size;

        for(int j=0;j<segNum;j++)
        {
            int curSeg = j;
            int nextSeg = (j + 1) % segNum;

            float innerX1, innerY1, innerX2, innerY2;
            float outerX1, outerY1, outerX2, outerY2;

            innerX1 = innerRadius * cosTheta[curSeg];
            innerY1 = innerRadius * sinTheta[curSeg];

            innerX2 = innerRadius * cosTheta[nextSeg];
            innerY2 = innerRadius * sinTheta[nextSeg];

            outerX1 = outerRadius * cosTheta[curSeg];
            outerY1 = outerRadius * sinTheta[curSeg];

            outerX2 = outerRadius * cosTheta[nextSeg];
            outerY2 = outerRadius * sinTheta[nextSeg];

            // draw a quad
            glBegin(GL_QUADS);
            // inner vertices
            setColor(c, innerAlpha * c.alphaF());
            glVertex2f(innerX1, innerY1);
            glVertex2f(innerX2, innerY2);

            // outer vertices
            setColor(c, outerAlpha * c.alphaF());
            glVertex2f(outerX2, outerY2);
            glVertex2f(outerX1, outerY1);
            glEnd();
        }
    }

    delete[] sinTheta;
    delete[] cosTheta;
}

void GLUtils::setColor(const QColor &c)
{
    glColor4f(c.redF(), c.greenF(), c.blueF(), c.alphaF());
}

void GLUtils::setColor(const QColor &c, const float& alpha)
{
    glColor4f(c.redF(), c.greenF(), c.blueF(), alpha);
}

GLenum GLUtils::getPolygonFillingStyle(PolygonFillingStyle style)
{
    switch(style)
    {
    case FILL:
        return GL_POLYGON;
        break;
    case DRAW:
        return GL_LINE_LOOP;
        break;
    default:
        return GL_POINTS;
        break;
    }
}

void GLUtils::renderCircle(float radius, float x, float y, float aspectRatio, int segNum, PolygonFillingStyle style)
{
    double theta = 0.0;
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(1.0 / aspectRatio, 1.0, 1.0);
    const float twoPI = 2.0 * PI;

    glBegin(getPolygonFillingStyle(style));
    {
        for(int i=0;i<segNum;i++)
        {
            theta = (float)i/(float)segNum*twoPI;
            glVertex2f(radius*cos(theta),radius*sin(theta));
        }
    }
    glEnd();
    glPopMatrix();
}

void GLUtils::fillCircle(float radius, float x, float y, float aspectRatio, int segNum)
{
    renderCircle(radius, x, y, aspectRatio, segNum, FILL);
}

void GLUtils::drawCircle(float radius, float x, float y, float aspectRatio, int segNum)
{
    renderCircle(radius, x, y, aspectRatio, segNum, DRAW);
}

void GLUtils::fillCircle(float radius, QPointF center, float aspectRatio, int segNum)
{
    renderCircle(radius, center.x(), center.y(), aspectRatio, segNum, FILL);
}

void GLUtils::drawCircle(float radius, QPointF center, float aspectRatio, int segNum)
{
    renderCircle(radius, center.x(), center.y(), aspectRatio, segNum, DRAW);
}

void GLUtils::drawPolygon(const vector<pair<float, float> > &polygon)
{
    glBegin(GL_LINE_LOOP);
    {
        vector<pair<float, float> >::const_iterator it = polygon.begin();
        while(it!=polygon.end())
        {
            glVertex3f((*it).first, (*it).second, 0.0);
            ++it;
        }
    }
    glEnd();
}

//==================================================
// complicated function for rendering non-convex polygon
//==================================================
#ifndef CALLBACK
#define CALLBACK
#endif
void CALLBACK beginCallback(GLenum which)
{
         glBegin(which);
}

void CALLBACK errorCallback(GLenum errorCode)
{
         const GLubyte *estring;
         estring = gluErrorString(errorCode);
         fprintf(stderr, "Tessellation Error: %s\n", estring);
         exit(0);
}

void CALLBACK endCallback(void)
{
         glEnd();
}

void CALLBACK vertexCallback(GLvoid *vertex)
{
         const GLdouble *pointer;
         pointer = (GLdouble *) vertex;
         glColor3dv(pointer+3);
         glVertex3dv(pointer);
}

void GLUtils::fillPolygon(const vector<pair<float, float> > &polygon)
{
    GLUtesselator *tobj;
    GLdouble* _polygon = new GLdouble[polygon.size()*3];
    for(unsigned int i=0;i<polygon.size();i++)
    {
        _polygon[i*3 + 0] = polygon.at(i).first;
        _polygon[i*3 + 1] = polygon.at(i).second;
        _polygon[i*3 + 2] = 0.0;
    }

    tobj = gluNewTess();
    gluTessCallback(tobj, GLU_TESS_VERTEX, (void (CALLBACK *) ())&glVertex3dv);
    gluTessCallback(tobj, GLU_TESS_BEGIN, (void (CALLBACK*) ())&glBegin);
    gluTessCallback(tobj, GLU_TESS_END, (void (CALLBACK*) ())&glEnd);

    glShadeModel(GL_SMOOTH);
    gluTessBeginPolygon(tobj, NULL);
    gluTessBeginContour(tobj);
        for(unsigned int i=0;i<polygon.size();i++)
            gluTessVertex(tobj, _polygon+i*3, _polygon+i*3);
    gluTessEndContour(tobj);
    gluTessEndPolygon(tobj);
    glEndList();
    gluDeleteTess(tobj);
}

void GLUtils::renderPolygon(const vector<pair<float, float> > &polygon, PolygonFillingStyle style)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    switch(style)
    {
    case FILL:
    {
        fillPolygon(polygon);
        break;
    }
    case DRAW:
    {
        drawPolygon(polygon);
        break;
    }
    default:
        break;
    }
    glPopMatrix();
}

void GLUtils::drawPolygon( vector<QPointF> p, float depth )
{
    glBegin(GL_LINE_LOOP);
    {
        for(unsigned int i=0;i<p.size();i++)
            glVertex3f(p.at(i).x(),p.at(i).y(), depth);
    }
    glEnd();
}

void GLUtils::drawPolygon( vector<QPointF> p )
{
    glBegin(GL_LINE_LOOP);
    {
        for(unsigned int i=0;i<p.size();i++)
            glVertex2f(p.at(i).x(),p.at(i).y());
    }
    glEnd();
}

void GLUtils::drawPolygon( QPolygonF p )
{
    glBegin(GL_LINE_LOOP);
    {
        for(int i=0;i<p.size();i++)
            glVertex2f(p.at(i).x(),p.at(i).y());
    }
    glEnd();
}

void GLUtils::fillPolygon( vector<QPointF> p )
{
#if 1
    GLUtesselator *tobj;
    GLdouble* polygon = new GLdouble[p.size()*3];
    for(unsigned int i=0;i<p.size();i++)
    {
        polygon[i*3 + 0] = p.at(i).x();
        polygon[i*3 + 1] = p.at(i).y();
        polygon[i*3 + 2] = 0.0;
    }

    tobj = gluNewTess();
    gluTessCallback(tobj, GLU_TESS_VERTEX, (void (CALLBACK *) ())&glVertex3dv);
    gluTessCallback(tobj, GLU_TESS_BEGIN, (void (CALLBACK*) ())&glBegin);
    gluTessCallback(tobj, GLU_TESS_END, (void (CALLBACK*) ())&glEnd);

    glShadeModel(GL_SMOOTH);
    gluTessBeginPolygon(tobj, NULL);
    gluTessBeginContour(tobj);
        for(unsigned int i=0;i<p.size();i++)
            gluTessVertex(tobj, polygon+i*3, polygon+i*3);
    gluTessEndContour(tobj);
    gluTessEndPolygon(tobj);
    glEndList();
    gluDeleteTess(tobj);
#else
    glBegin(GL_POLYGON);
    for(int i=0;i<p.size();i++)
    {
        glVertex2f(p.at(i).x(), p.at(i).y());
    }
    glEnd();
#endif
}

void GLUtils::fillPolygon( const QPolygonF& p )
{
#if 1
    GLUtesselator *tobj;
    GLdouble* polygon = new GLdouble[p.size()*3];
    for(int i=0;i<p.size();i++)
    {
        polygon[i*3 + 0] = p.at(i).x();
        polygon[i*3 + 1] = p.at(i).y();
        polygon[i*3 + 2] = 0.0;
    }

    tobj = gluNewTess();
    gluTessCallback(tobj, GLU_TESS_VERTEX, (void (CALLBACK *) ())&glVertex3dv);
    gluTessCallback(tobj, GLU_TESS_BEGIN, (void (CALLBACK*) ())&glBegin);
    gluTessCallback(tobj, GLU_TESS_END, (void (CALLBACK*) ())&glEnd);

    glShadeModel(GL_SMOOTH);
    gluTessBeginPolygon(tobj, NULL);
    gluTessBeginContour(tobj);
        for(int i=0;i<p.size();i++)
            gluTessVertex(tobj, polygon+i*3, polygon+i*3);
    gluTessEndContour(tobj);
    gluTessEndPolygon(tobj);
    glEndList();
    gluDeleteTess(tobj);
#else
    glBegin(GL_POLYGON);
    for(int i=0;i<p.size();i++)
    {
        glVertex2f(p.at(i).x(), p.at(i).y());
    }
    glEnd();
#endif
}

void GLUtils::drawCurve( vector<QPointF>& p )
{
    glBegin(GL_LINE_STRIP);
    {
        for(unsigned int i=0;i<p.size();i++)
            glVertex2f(p.at(i).x(),p.at(i).y());
    }
    glEnd();
}

void GLUtils::drawCurve( const QPointF* p, int segmentNum)
{
    glBegin(GL_LINE_STRIP);
    {
        for(int i=0;i<segmentNum;i++)
            glVertex2f(p[i].x(),p[i].y());
    }
    glEnd();
}

void GLUtils::drawCurve( const QPointF* p, int segmentNum, float depth )
{
    glBegin(GL_LINE_STRIP);
    {
        for(int i=0;i<segmentNum;i++)
            glVertex3f(p[i].x(),p[i].y(), depth);
    }
    glEnd();
}

void GLUtils::renderRectangle(float x1, float y1, float x2, float y2, PolygonFillingStyle style)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glBegin(getPolygonFillingStyle(style));
    {
        glVertex2f(x1, y1);
        glVertex2f(x1, y2);
        glVertex2f(x2, y2);
        glVertex2f(x2, y1);
    }
    glEnd();
    glPopMatrix();
}

void GLUtils::drawRectangle( float x1, float y1, float x2, float y2 )
{
    renderRectangle(x1, y1, x2, y2, DRAW);
}

void GLUtils::drawRectangle( const QRectF& r )
{
    drawRectangle(r.left(), r.top(), r.right(), r.bottom());
}

void GLUtils::drawRectangle( const QPointF& topLeft, const QPointF& bottomRight )
{
    drawRectangle(topLeft.x(), topLeft.y(), bottomRight.x(), bottomRight.y());
}

void GLUtils::fillRectangle( float x1, float y1, float x2, float y2 )
{
    renderRectangle(x1, y1, x2, y2, FILL);
}

void GLUtils::fillRectangle( const QRectF& r )
{
    fillRectangle(r.left(), r.top(), r.right(), r.bottom());
}

void GLUtils::fillRectangle( const QPointF& topLeft, const QPointF &bottomRight)
{
    fillRectangle(topLeft.x(), topLeft.y(), bottomRight.x(), bottomRight.y());
}

void GLUtils::drawSquare(float x, float y, float size, float aspectRatio)
{
    renderSquare(x, y, size, aspectRatio, DRAW);
}

void GLUtils::fillSquare(float x, float y, float size, float aspectRatio)
{
    renderSquare(x, y, size, aspectRatio, FILL);
}

void GLUtils::renderSquare(float x, float y, float size, float aspectRatio, PolygonFillingStyle style)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(x, y, 0.0);
    glScalef(1.0 / aspectRatio, 1.0, 1.0);
    glBegin(getPolygonFillingStyle(style));
    {
        glVertex2f(0, 0);
        glVertex2f(size, 0);
        glVertex2f(size, size);
        glVertex2f(0, size);
    }
    glEnd();
    glPopMatrix();
}


void GLUtils::drawLine( float x1, float y1, float x2, float y2 )
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glBegin(GL_LINES);
    {
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    }
    glEnd();
    glPopMatrix();
}

void GLUtils::drawLine( QPointF p1, QPointF p2 )
{
    drawLine(p1.x(), p1.y(), p2.x(), p2.y());
}

void GLUtils::drawPoint( QPointF p, float pointSize )
{
    drawPoint(p.x(), p.y(), pointSize);
}

void GLUtils::drawPoint( float x, float y, float pointSize )
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glPointSize(pointSize);
    glBegin(GL_POINTS);
    {
        glVertex2f(x, y);
    }
    glEnd();
    glPopMatrix();
}

void GLUtils::drawOval( float x, float y, float a, float b, float aspectRatio, int segNum /*= circleSegNum*/ )
{
    double theta = 0.0;
    glBegin(GL_LINE_LOOP);
    {
        for(int i=0;i<segNum;i++)
        {
            theta = (float)i/(float)segNum*2.0*PI;
            glVertex2f(x+a*cos(theta)/aspectRatio,y+b*sin(theta));
        }
    }
    glEnd();
}

void GLUtils::drawOval( QPointF center, float a, float b, float aspectRatio, int segNum /*= circleSegNum*/ )
{
    double theta = 0.0;
    glBegin(GL_LINE_LOOP);
    {
        for(int i=0;i<segNum;i++)
        {
            theta = (float)i/(float)segNum*2.0*PI;
            glVertex2f(center.x()+a*cos(theta)/aspectRatio,center.y()+b*sin(theta));
        }
    }
    glEnd();

}

void GLUtils::fillOval( float x, float y, float a, float b, float aspectRatio, int segNum /*= circleSegNum*/ )
{
    double theta = 0.0;
    glBegin(GL_POLYGON);
    {
        for(int i=0;i<segNum;i++)
        {
            theta = (float)i/(float)segNum*2.0*PI;
            glVertex2f(x+a*cos(theta)/aspectRatio,y+b*sin(theta));
            //glVertex2f(x+a*cos(theta),y+b*sin(theta));
        }
    }
    glEnd();

}

void GLUtils::fillOval( QPointF center, float a, float b, float aspectRatio, int segNum /*= circleSegNum*/ )
{
    double theta = 0.0;
    glBegin(GL_POLYGON);
    {
        for(int i=0;i<segNum;i++)
        {
            theta = (float)i/(float)segNum*2.0*PI;
            glVertex2f(center.x()+a*cos(theta)/aspectRatio,center.y()+b*sin(theta));
        }
    }
    glEnd();
}


// fill ellipse rotated by given angle
void GLUtils::fillEllipse( float x, float y, float a, float b, float aspectRatio, float angle, int segNum /*= circleSegNum*/ )
{
    double theta = 0.0;
    glBegin(GL_POLYGON);
    {
        for(int i=0;i<segNum;i++)
        {
            theta = (float)i/(float)segNum*2.0*PI;
            float xPos = a*cos(theta);
            float yPos = b*sin(theta);

            float pX = cos(-angle)*xPos + sin(-angle)*yPos;
            float pY = cos(-angle)*yPos - sin(-angle)*xPos;

            glVertex2f(x+pX/aspectRatio,y+pY);
        }
    }
    glEnd();

}

GLuint GLUtils::generateTexture(const QImage &img, GLuint &tex)
{
    int width = img.width();
    int height = img.height();
    cout<<"Image size: "<<width<<"x"<<height<<endl;
    cout<<"Generating image texture..."<<endl;
    GLubyte *m = new GLubyte[width * height * 4];

    int i, j;
    //float c;

    for (i = 0; i < height; i++) {
        int y = height - 1 - i;
        for (j = 0; j < width; j++) {
            int x = j;
            QRgb value = img.pixel(j, i);
            int r = qRed(value);
            int g = qGreen(value);
            int b = qBlue(value);
            int a = qAlpha(value);
            int baseIdx = (y * width + x) * 4;
            //float randVaue = (float)rand()/(float)RAND_MAX;
            m[baseIdx+ 0] = (GLubyte) r;
            m[baseIdx+ 1] = (GLubyte) g;
            m[baseIdx+ 2] = (GLubyte) b;
            m[baseIdx+ 3] = (GLubyte) a;
        }
    }

    glEnable(GL_TEXTURE_2D);
    glDeleteTextures(1, &tex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width,
                 height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 m);
    glDisable(GL_TEXTURE_2D);

    cout<<"Image texture ID: "<<tex<<endl;

    delete[] m;

    return tex;
}

GLuint GLUtils::generateTexture(const RGBAImage &img, GLuint &tex)
{
    int width = img.width();
    int height = img.height();
    cout<<"Image size: "<<width<<"x"<<height<<endl;
    cout<<"Generating image texture..."<<endl;
    GLfloat *m = new GLfloat[width * height * 4];

    int i, j;
    //float c;

    for (i = 0; i < height; i++) {
        int y = i;
        for (j = 0; j < width; j++) {
            int x = j;
            float r = img(x, y, 0);
            float g = img(x, y, 1);
            float b = img(x, y, 2);
            float a = img(x, y, 3);
            int baseIdx = (y * width + x) * 4;
            //float randVaue = (float)rand()/(float)RAND_MAX;
            m[baseIdx+ 0] = r;
            m[baseIdx+ 1] = g;
            m[baseIdx+ 2] = b;
            m[baseIdx+ 3] = a;
        }
    }

    glEnable(GL_TEXTURE_2D);
    glDeleteTextures(1, &tex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, width,
                 height, 0, GL_RGBA, GL_FLOAT,
                 m);
    glDisable(GL_TEXTURE_2D);

    cout<<"Image texture ID: "<<tex<<endl;

    delete[] m;

    return tex;
}
