#ifndef OCTREEBASEDVISUALHULL_H
#define OCTREEBASEDVISUALHULL_H

#include "silhouettebasedreconstructor.h"
#include "voxelarraymodel.h"

#include "binaryimage.h"

#include "geometryutils.hpp"
using namespace GeometryUtils;

#include <map>
#include <list>
#include <queue>
#include <cfloat>
#include <cmath>
using namespace std;

#include <QApplication>

class OctreeBasedVisualHullReconstructor : public SilhouetteBasedReconstructor
{
public:
    OctreeBasedVisualHullReconstructor();
    ~OctreeBasedVisualHullReconstructor();

    virtual void performReconstruction();
    virtual void outputModel();

    void setSplittingThreshold(double threshd) { _splittingThreshold = threshd; }
    void setCutoffVoxelSize(double size) { _minVoxelSize = size; }

private:
    inline list<Voxel> splitVoxel(const Voxel&);

private:
    inline void makeCorners(DblPoint3D* pts, float, float, float, float, float, float);
    inline void projectCorners(const DblMatrix& mat, DblPoint3D* pts, DblPoint3D* projPts, const DblPoint3D&, const DblPoint3D&, size_t pointNumber);
    inline void approximateFootprints(DblPoint3D* pts, int&, int&, int&, int&, size_t);
    inline bool consistencyEvaluation(size_t, size_t, size_t, size_t, size_t, bool&, bool&, float adaptiveFactor);

private:
    VoxelArrayModel* _model;
    double _splittingThreshold;
    double _minVoxelSize;
};

#endif // OCTREEBASEDVISUALHULL_H
