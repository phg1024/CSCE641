#ifndef VISUALHULLRECONSTRUCTOR_H
#define VISUALHULLRECONSTRUCTOR_H

#include "global_definitions.h"
#include "silhouettebasedreconstructor.h"

#include "voxelarraymodel.h"
#include "binaryimage.h"

#include "geometryutils.hpp"
using namespace GeometryUtils;

#include <map>
#include <list>
#include <cfloat>
#include <cmath>
using namespace std;

#include <QApplication>
#include <QGLShaderProgram>
#include <QGLPixelBuffer>
#include <QGLFramebufferObject>

class VisualHullReconstructor : public SilhouetteBasedReconstructor
{
public:
    VisualHullReconstructor();
    ~VisualHullReconstructor();

    virtual void performReconstruction();
    virtual void outputModel();

    void setVoxelSize(double size){voxel_size = size;}
    void setInsideThreshold(double threshd){ inside_threshold = threshd; }

private:
    void CPUReconstruction();
    void GPUReconstruction();

private:
    inline void makeCorners(DblPoint3D* pts, float, float, float, float, float, float);
    inline void projectCorners(const DblMatrix& mat, DblPoint3D* pts, DblPoint3D* projPts, const DblPoint3D&, const DblPoint3D&, size_t pointNumber);
    inline void approximateFootprints(DblPoint3D* pts, int&, int&, int&, int&, size_t);
    inline bool evaluateFootprint(size_t, size_t, size_t, size_t, size_t, bool&);

private:
    double voxel_size;
    double inside_threshold;

    VoxelArrayModel *vaModel;
};

#endif // VISUALHULLRECONSTRUCTOR_H
