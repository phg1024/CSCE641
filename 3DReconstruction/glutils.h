#ifndef GLUTILS_H
#define GLUTILS_H

#include "GL/glew.h"
#include "GL/glut.h"

#include <QPoint>
#include <QPointF>
#include <QColor>
#include <QPolygonF>
#include <QRectF>

#include <QImage>
#include "rgbaimage.h"

#include <cmath>
#include <map>
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <iostream>
using namespace std;

//! OpenGL drawing utility for frequently used 2D geometry

class GLUtils
{
public:
    // rendering related
    static void setColor(const QColor&);
    static void setColor(const QColor&, const float&);

    static void drawPoint(float x, float y, float pointSize);
    static void drawPoint(QPointF p, float pointSize);
    static void drawLine(float x1, float y1, float x2, float y2);
    static void drawLine(QPointF p1, QPointF p2);

    static void drawRectangle(float x1, float y1, float x2, float y2);
    static void drawRectangle(const QPointF& topLeft, const QPointF& bottomRight);
    static void drawRectangle(const QRectF& r);
    static void fillRectangle(const QPointF& topLeft, const QPointF& bottomRight);
    static void fillRectangle(float x1, float y1, float x2, float y2);
    static void fillRectangle(const QRectF& r);

    static void drawSquare(float x, float y, float size, float aspectRatio  = 1.0);
    static void fillSquare(float x, float y, float size, float aspectRatio  = 1.0);

    // curves
    static void drawCurve(vector<QPointF>& p);
    static void drawCurve(const QPointF* p, int segmentNum);
    static void drawCurve(const QPointF* p, int segmentNum, float depth);

    // polygons
    static void drawPolygon(vector<QPointF> p);
    static void drawPolygon(QPolygonF p);
    static void drawPolygon(vector<QPointF> p, float depth);
    static void fillPolygon(vector<QPointF> p);
    static void fillPolygon( const QPolygonF& p);

    static void drawCircle(float radius, QPointF center,  float aspectRatio  = 1.0, int segNum = CIRCLE_SEG_NUM);
    static void fillCircle(float radius, QPointF center,  float aspectRatio  = 1.0, int segNum = CIRCLE_SEG_NUM);
    static void drawCircle(float radius, float x, float y,  float aspectRatio  = 1.0, int segNum = CIRCLE_SEG_NUM);
    static void fillCircle(float radius, float x, float y,  float aspectRatio  = 1.0, int segNum = CIRCLE_SEG_NUM);
    static void drawGradient(const QPointF&, const QColor&, const float& size, const float& aspectRatio, int layerNum = 20, int segNum = 36);

    static void drawOval(float x, float y, float a, float b, float aspectRatio  = 1.0, int segNum = CIRCLE_SEG_NUM);
    static void drawOval(QPointF center, float a, float b, float aspectRatio  = 1.0, int segNum = CIRCLE_SEG_NUM);
    static void fillOval(float x, float y, float a, float b, float aspectRatio, int segNum = CIRCLE_SEG_NUM);
    static void fillOval(QPointF center, float a, float b, float aspectRatio  = 1.0, int segNum = CIRCLE_SEG_NUM);

    static void fillEllipse(float x, float y, float a, float b, float aspectRatio, float angle, int segNum = CIRCLE_SEG_NUM);

    // texture related
    static GLuint generateTexture(const QImage&, GLuint& tex);
    static GLuint generateTexture(const RGBAImage&, GLuint& tex);

private:
    enum PolygonFillingStyle{
        FILL,
        DRAW
    };

    static const int CIRCLE_SEG_NUM = 108;

    static inline GLenum getPolygonFillingStyle(PolygonFillingStyle style);

    static void renderCircle(float radius, float x, float y, float aspectRatio, int segNum, PolygonFillingStyle style);
    static void renderRectangle(float x1, float y1, float x2, float y2, PolygonFillingStyle style);
    static void renderPolygon(const vector< pair<float, float> >& polygon, PolygonFillingStyle);
    static void drawPolygon(const vector< pair<float, float> >& polygon);
    static void fillPolygon(const vector< pair<float, float> >& polygon);

    static void renderSquare(float x, float y, float size, float aspectRatio, PolygonFillingStyle style);

};

#endif // GLUTILS_H
