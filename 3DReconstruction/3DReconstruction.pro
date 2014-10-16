#-------------------------------------------------
#
# Project created by QtCreator 2011-10-16T03:47:43
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = 3DReconstruction
TEMPLATE = app
LIBS    += -lglut \
           -lGLEW

SOURCES += main.cpp\
        mainwindow.cpp \
    visualhullreconstructor.cpp \
    reconstructor.cpp \
    voxelcoloringreconstructor.cpp \
    rgbaimage.cpp \
    imageviewer.cpp \
    grayscaleimage.cpp \
    reconstructionengine.cpp \
    controlpanel.cpp \
    silhouettebasedreconstructor.cpp \
    binaryimage.cpp \
    glTrackball.cpp \
    glutils.cpp \
    glEnv.cpp \
    gl3dcanvas.cpp \
    glcanvaswindow.cpp \
    abstractmodel.cpp \
    geometryutils.cpp \
    modelviewer.cpp \
    depthmap.cpp \
    octreebasedvisualhull.cpp \
    voxelarraymodel.cpp \
    helpdialog.cpp

HEADERS  += mainwindow.h \
    visualhullreconstructor.h \
    reconstructor.h \
    voxelcoloringreconstructor.h \
    utility.hpp \
    util_common.h \
    rgbaimage.h \
    mathutil.hpp \
    imageviewer.h \
    grayscaleimage.h \
    reconstructionengine.h \
    controlpanel.h \
    silhouettebasedreconstructor.h \
    binaryimage.h \
    glTrackball.h \
    glutils.h \
    glEnv.h \
    gl3dcanvas.h \
    glcanvaswindow.h \
    global_definitions.h \
    array2d.hpp \
    abstractimage.hpp \
    geometryutils.hpp \
    modelviewer.h \
    depthmap.h \
    octreebasedvisualhull.h \
    voxelarraymodel.h \
    helpdialog.h

FORMS    += mainwindow.ui \
    helpdialog.ui

OTHER_FILES += \
    project_arch.txt \
    TODO.txt
