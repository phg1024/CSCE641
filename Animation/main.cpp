#include <QtGui/QApplication>

#include "GL/glew.h"
#include "GL/glut.h"

#include "mainwindow.h"
#include "humanbodymodel.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

int main(int argc, char *argv[])
{
#ifdef Q_WS_X11
    XInitThreads();
#endif
    QApplication a(argc, argv);

    glutInit(&argc, argv);

    MainWindow w;
    w.resize(1024, 768);
    w.show();

    return a.exec();
}
