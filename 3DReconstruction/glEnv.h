#ifndef _GLENV_H
#define _GLENV_H

#ifdef Q_OS_WIN
#include "GL/glew.h"
#endif
#ifdef Q_OS_LINUX
#include <GL/glew.h>
#endif

#include <QtOpenGL>

void checkGLError(const char *situation = 0);

#endif
