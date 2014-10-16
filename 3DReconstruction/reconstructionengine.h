#ifndef RECONSTRUCTIONENGINE_H
#define RECONSTRUCTIONENGINE_H

// worker class to perform 3d reconstruction by invoking reconstructor
#include "visualhullreconstructor.h"
#include "voxelcoloringreconstructor.h"
#include "octreebasedvisualhull.h"

#include <QString>

class ReconstructionEngine : public QObject
{
    Q_OBJECT
public:
    enum EngineType{
        VisualHull,
        VoxelColoring,
        OctreeBasedVH,
        Unknown
    };
    ReconstructionEngine(EngineType t);
    ~ReconstructionEngine();

    void reconstructModel(const string& desFilename, const string& modelFilename);

    static EngineType interpretEngineType(QString);

signals:
    void sig_progress(double);

public:
    void setConsistencyThreshold(double threshd){ consistency_threshold = threshd; }
    void setBackgroundThreshold(double threshd){ background_threshold = threshd; }
    void setVoxelSize(double size){ voxel_size = size; }
    void setTraversalDirection(int dir){ traversal_direction = dir; }
    void setInsideThreshold(double threshd){ inside_threshold = threshd; }
    void setSplitThreshold(double threshd) { split_threshold = threshd; }
    void setCutoffVoxelSize(double size) { cutoff_voxel_size = size; }

private:
    EngineType _type;
    Reconstructor* _r;

private:
    // for voxel coloring
    double consistency_threshold;
    double background_threshold;
    double voxel_size;
    int traversal_direction;

    // for visual hull
    double inside_threshold;

    // for octree-based visual hull
    double split_threshold;
    double cutoff_voxel_size;
};

#endif // RECONSTRUCTIONENGINE_H
