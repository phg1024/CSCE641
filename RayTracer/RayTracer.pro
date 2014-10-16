#-------------------------------------------------
#
# Project created by QtCreator 2011-12-01T17:05:22
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = RayTracer
TEMPLATE = app
QMAKE_CXXFLAGS += -fopenmp
LIBS += -lglut \
        -lGLEW \
        -lgomp

SOURCES += main.cpp\
        mainwindow.cpp \
    glTrackball.cpp \
    glEnv.cpp \
    glcanvaswindow.cpp \
    gl3dcanvas.cpp \
    geometryutils.cpp \
    modelviewer.cpp \
    controlpanel.cpp \
    shape.cpp \
    camerainfo.cpp \
    rgbaimage.cpp \
    raytracer.cpp \
    scene.cpp \
    sceneparser.cpp \
    lightsource.cpp \
    imageviewer.cpp

HEADERS  += mainwindow.h \
    utility.hpp \
    util_common.h \
    matrixutil.hpp \
    mathutil.hpp \
    glTrackball.h \
    glEnv.h \
    glcanvaswindow.h \
    gl3dcanvas.h \
    geometryutils.hpp \
    modelviewer.h \
    controlpanel.h \
    shape.h \
    camerainfo.h \
    rgbaimage.h \
    abstractimage.hpp \
    raytracer.h \
    scene.h \
    sceneparser.h \
    lightsource.h \
    color.hpp \
    imageviewer.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    TODO.txt

RESOURCES += \
    rec.qrc
