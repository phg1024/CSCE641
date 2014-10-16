#-------------------------------------------------
#
# Project created by QtCreator 2011-11-10T16:42:27
#
#-------------------------------------------------

QT       += core gui opengl phonon

TARGET = Animation
TEMPLATE = app
LIBS += -lglut \
        -lGLEW \
	-L levmar-2.5/build \
        -L../Animation/levmar-2.5/build -llevmar \
        -llapack \
        -lblas \

SOURCES += main.cpp\
        mainwindow.cpp \
    humanbodymodel.cpp \
    glutils.cpp \
    glTrackball.cpp \
    glEnv.cpp \
    glcanvaswindow.cpp \
    gl3dcanvas.cpp \
    geometryutils.cpp \
    modelviewer.cpp \
    controlpanel.cpp \
    humanbodymodelrenderer.cpp \
    asffileparser.cpp \
    amcfileparser.cpp \
    forwardkinematicsoperator.cpp \
    inversekinematicsoperator.cpp

HEADERS  += mainwindow.h \
    humanbodymodel.h \
    utility.hpp \
    util_common.h \
    glutils.h \
    glTrackball.h \
    glEnv.h \
    glcanvaswindow.h \
    gl3dcanvas.h \
    geometryutils.hpp \
    modelviewer.h \
    mathutil.hpp \
    matrixutil.hpp \
    controlpanel.h \
    humanbodymodelrenderer.h \
    asffileparser.h \
    amcfileparser.h \
    forwardkinematicsoperator.h \
    inversekinematicsoperator.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    TODO.txt
