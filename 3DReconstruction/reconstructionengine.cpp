#include "reconstructionengine.h"

ReconstructionEngine::ReconstructionEngine(EngineType t):
    _type(t),
    _r(0)
{
    switch(t)
    {
    case VisualHull:
    {
        _r = new VisualHullReconstructor();
        break;
    }
    case OctreeBasedVH:
    {
        _r = new OctreeBasedVisualHullReconstructor();
        break;
    }
    case VoxelColoring:
    {
        _r = new VoxelColoringReconstructor();
        break;
    }
    default:
    {
        _r = 0;
        break;
    }
    }
    if( _r )
    {
        connect(_r, SIGNAL(sig_progress(double)), this, SIGNAL(sig_progress(double)));
    }
}

ReconstructionEngine::~ReconstructionEngine()
{
    if(_r!=0)
        delete _r;
}

void ReconstructionEngine::reconstructModel(const string &desFilename, const string &modelFilename)
{
    if(_r!=0)
    {
        _r->setDescriptionFile(desFilename);
        _r->setModelFile(modelFilename);

        // reconstructor specific settings
        switch( _type )
        {
        case VisualHull:
        {
            VisualHullReconstructor *_vhr = dynamic_cast<VisualHullReconstructor*>(_r);
            if(_vhr != 0)
            {
                _vhr->setVoxelSize(voxel_size);
                _vhr->setInsideThreshold(inside_threshold);
            }
            break;
        }
        case OctreeBasedVH:
        {
            OctreeBasedVisualHullReconstructor *_obvhr = dynamic_cast<OctreeBasedVisualHullReconstructor*>(_r);
            if(_obvhr != 0)
            {
                _obvhr->setSplittingThreshold(split_threshold);
                _obvhr->setCutoffVoxelSize(cutoff_voxel_size);
            }
            break;
        }
        case VoxelColoring:
        {
            VoxelColoringReconstructor *_vcr = dynamic_cast<VoxelColoringReconstructor*>(_r);
            if(_vcr != 0)
            {
                _vcr->setBackgroundThreshold(background_threshold);
                _vcr->setConsistencyThreshold(consistency_threshold);
                _vcr->setVoxelSize(voxel_size);
                _vcr->setTraversalDirection(traversal_direction);
            }
            break;
        }
        default:
            return;
        }

        _r->performReconstruction();
        _r->outputModel();
    }
}

ReconstructionEngine::EngineType ReconstructionEngine::interpretEngineType(QString t)
{
    if(t.toLower() == "visual hull")
        return VisualHull;
    else if (t.toLower() == "voxel coloring")
        return VoxelColoring;
    else if (t.toLower() == "octree-based vh")
        return OctreeBasedVH;
    else
        return Unknown;
}
