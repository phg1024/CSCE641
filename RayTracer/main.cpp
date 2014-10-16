#include <QtGui/QApplication>

#include "GL/glew.h"
#include "GL/glut.h"

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    glutInit(&argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
