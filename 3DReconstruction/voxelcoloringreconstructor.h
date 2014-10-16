#ifndef VOXELCOLORINGRECONSTRUCTOR_H
#define VOXELCOLORINGRECONSTRUCTOR_H

#include "global_definitions.h"

#include "silhouettebasedreconstructor.h"

#include "voxelarraymodel.h"

#include "depthmap.h"

#include "geometryutils.hpp"

using namespace GeometryUtils;

#include <map>
#include <list>
#include <cfloat>
#include <cmath>
using namespace std;

#include <QApplication>

class VoxelColoringReconstructor : public SilhouetteBasedReconstructor
{
public:
    VoxelColoringReconstructor();
    ~VoxelColoringReconstructor();

    virtual void performReconstruction();
    virtual void outputModel();

    void setBackgroundThreshold(double threshd){ background_threshold = threshd; }
    void setConsistencyThreshold(double threshd){ consistency_threshold = threshd; }
    void setVoxelSize(double size){ voxel_size = size; }
    void setTraversalDirection(int dir){ traversal_direction = dir; }

private:
    // use this to store pixel pointers
    typedef struct {
        int x, y;
        int imgIdx;
    } PixelInfo;

private:
    inline void makeCorners(DblPoint3D* pts, float, float, float, float, float, float);
    inline void projectCorners(const DblMatrix& mat, DblPoint3D* pts, DblPoint3D* projPts, const DblPoint3D&, const DblPoint3D&, size_t pointNumber);
    inline void approximateFootprints(DblPoint3D* pts, int&, int&, int&, int&, size_t);
    inline void getPixels(const int&, const int&, const int&, const int&, size_t, BinaryImage*, list<PixelInfo>&);
    inline bool isBackgroundPixel(const RGBAPixel& p);
    inline void consistencyTest(const list<PixelInfo>&, double&, double&, double&, double&, double&, double&);
    inline void markPixels(const list<PixelInfo>&, BinaryImage*);

private:
    double consistency_threshold;
    double background_threshold;
    double voxel_size;
    int traversal_direction;

    VoxelArrayModel* vaModel;
};

#endif // VOXELCOLORINGRECONSTRUCTOR_H
