#include "octreebasedvisualhull.h"
#include "utility.hpp"

using namespace Utils;

OctreeBasedVisualHullReconstructor::OctreeBasedVisualHullReconstructor():
    _model(0),
    _splittingThreshold(0.5),
    _minVoxelSize(1.0)
{
}

OctreeBasedVisualHullReconstructor::~OctreeBasedVisualHullReconstructor()
{
    if( _model )
        delete _model;
}

void OctreeBasedVisualHullReconstructor::performReconstruction()
{
    SilhouetteBasedReconstructor::performReconstruction();

    cout << "performing octree based visual hull reconstruction ..." << endl;
    cout << "cut off voxel size = " << _minVoxelSize << endl;
    cout << "splitting threshold = " << _splittingThreshold << endl;

    size_t xSize = abs(_xMax - _xMin);
    size_t ySize = abs(_yMax - _yMin);
    size_t zSize = abs(_zMax - _zMin);

    float scaleX, scaleY, scaleZ;
    size_t maxDim = (xSize > ySize)?xSize:ySize;
    size_t minDim = (xSize < ySize)?xSize:ySize;
    maxDim = (maxDim > zSize)?maxDim:zSize;
    minDim = (minDim < zSize)?minDim:zSize;
    scaleX = (float) xSize / (float) maxDim, scaleY = (float) ySize / (float) maxDim, scaleZ = (float) zSize / (float) maxDim;

    double cutoffThreshold = _minVoxelSize / minDim;

    DblPoint3D scaleVector(fabs(_xMax - _xMin), fabs(_yMax - _yMin), fabs(_zMax - _zMin));
    cout << "scale vector = " << scaleVector.x() << ", "
         << scaleVector.y() << ", "
         << scaleVector.z() << endl;

    DblPoint3D translationVector(min(_xMax, _xMin),
                                 min(_yMax, _yMin),
                                 min(_zMax, _zMin));
    cout << "translation vector = " << translationVector.x() << ", "
         << translationVector.y() << ", "
         << translationVector.z() << endl;

    _model = new VoxelArrayModel(scaleX, scaleY, scaleZ);

    Voxel boundingBox;
    boundingBox.xMin = -0.5, boundingBox.xMax = 0.5;
    boundingBox.yMin = -0.5, boundingBox.yMax = 0.5;
    boundingBox.zMin = -0.5, boundingBox.zMax = 0.5;

#if OUT_PROJ_IMG
    vector<QImage> projImages;
    for(size_t i=0;i<_inputSize;i++)
    {
        QImage img = _inputImages[i].toQImage();
        projImages.push_back(img);
    }
#endif

    // loop over every pixel and try to color consistent ones

    // candidate voxels
    queue<Voxel> candidateList;
    candidateList.push(boundingBox);
    size_t maxQueueSize = 0;

    //for(size_t z=0; z<_resolution; z++)
    // z from 1 to 0
    while(!candidateList.empty())
    {
        if(candidateList.size() >= maxQueueSize) maxQueueSize = candidateList.size();

        //cout << "candidate list size = " << candidateList.size() << endl;
        Voxel v = candidateList.front();
        candidateList.pop();

        emit sig_progress(( maxQueueSize - candidateList.size()) / (double) maxQueueSize);

        float x0, x1, y0, y1, z0, z1;
        x0 = v.xMin, x1 = v.xMax;
        y0 = v.yMin, y1 = v.yMax;
        z0 = v.zMin, z1 = v.zMax;

        float curSize = abs(x1 - x0);
        const float steepness = 0.5;
        // impose strict coverage condition for higher levels
        // for lower levels, coverage condition are loosen to avoid too much fine details
        float adaptiveFactor = 1.0 - powf(cutoffThreshold / curSize, steepness);//powf(2.0 * curSize, 0.125);

        //cout << "processing voxel @ " << x << ", " << y << ", " << z << endl;

        DblPoint3D corners[8];
        DblPoint3D footprints[8];

        makeCorners(corners, x0, x1, y0, y1, z0, z1);

        bool needSplit = false;
        bool outsideVoxel = false;
        bool hasIntersection = false;
        bool insideVoxel = true;

        // project the voxel to image plane
        for(size_t i=0;i<_inputSize;i++)
        {
            projectCorners(_projMat[i], corners, footprints, scaleVector, translationVector, 8);

            // use a rectangle footprint to approximate the real footprint
            // (u, v) is image space coordinates
            int minU, maxU, minV, maxV;
            approximateFootprints(footprints, minU, maxU, minV, maxV, 8);
            int w = _inputImages[i].width(), h = _inputImages[i].height();

            // not int image plane
            if( maxU < 0 || minU >= w
                    || maxV < 0 || minV >= h )
            {
                outsideVoxel = true;
                break;
            }
            else
            {
                // restrict the footprint to the image plane
                minU = clamp<int>(0, w - 1, minU); maxU = clamp<int>(0, w - 1, maxU);
                minV = clamp<int>(0, h - 1, minV); maxV = clamp<int>(0, h - 1, maxV);
            }

            bool isInside;

            needSplit |= consistencyEvaluation(minU, maxU, minV, maxV, i, hasIntersection, isInside, adaptiveFactor);

            insideVoxel &= isInside;

            if(!hasIntersection)
            {
                outsideVoxel = true;
                break;
            }
        }

        // completely outside silhouette
        if( outsideVoxel )
            continue;

        if( !insideVoxel )
        {
            if( needSplit )
            {
                //cout << v.xMax - v.xMin << endl;
                if( fabs(v.xMax - v.xMin) > cutoffThreshold )
                {
                    // maybe part of this voxel is valid
                    list<Voxel> childrenVoxels = splitVoxel(v);
                    list<Voxel>::iterator cit = childrenVoxels.begin();
                    while( cit != childrenVoxels.end() )
                    {
                        candidateList.push((*cit));
                        cit++;
                    }
                }
            }
            else
            {
                // highly probable boundary voxels
                v.r = 175, v.g = 175, v.b = 175, v.a = 255;
                _model->addVoxel(v);
            }
        }
    }

    cout << _model->voxelNumber() << " voxels painted." << endl;
    cout << "max queue size = " << maxQueueSize << endl;

    cout << "visual hull done." << endl;
}

void OctreeBasedVisualHullReconstructor::outputModel()
{
    _model->write(_modelFilename);
}

void OctreeBasedVisualHullReconstructor::makeCorners(DblPoint3D* pts, float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
    for(size_t i=0;i<2;i++)
    {
        float x = (i == 0)? minX : maxX;
        for(size_t j=0;j<2;j++)
        {
            float y = (j == 0)? minY : maxY;
            for(size_t k=0;k<2;k++)
            {
                float z = (k == 0)? minZ : maxZ;
                size_t idx = i * 4 + j * 2 + k;
                pts[idx] = DblPoint3D(x, y, z);
            }
        }
    }
}

void OctreeBasedVisualHullReconstructor::projectCorners(const DblMatrix& mat,
                                                        DblPoint3D *pts,
                                                        DblPoint3D *projPts,
                                                        const DblPoint3D& scaleVec,
                                                        const DblPoint3D& translationVec,
                                                        size_t pointNumber)
{
    // valid only for alien data set
    const DblPoint3D shift(0.5, 0.5, 0.5);

    for(size_t i=0;i<pointNumber;i++)
    {
        DblPoint3D p = pts[i] + shift;
        p.x() *= scaleVec.x();
        p.y() *= scaleVec.y();
        p.z() *= scaleVec.z();

        p = p + translationVec;

        projPts[i] = DblPoint3D::fromVector(mat * DblPoint4D(p, 1.0).toVector());
        projPts[i].x() /= projPts[i].z();
        projPts[i].y() /= projPts[i].z();
    }
}

void OctreeBasedVisualHullReconstructor::approximateFootprints(GeometryUtils::DblPoint3D *pts,
                                                               int &minU, int &maxU, int &minV, int &maxV,
                                                               size_t numFootprints)
{
    float u[2], v[2];
    u[0] = -FLT_MAX, u[1] = FLT_MAX;
    v[0] = -FLT_MAX, v[1] = FLT_MAX;
    for(size_t i=0;i<numFootprints;i++)
    {
        if(pts[i].x() > u[0]) u[0] = pts[i].x();
        if(pts[i].x() < u[1]) u[1] = pts[i].x();
        if(pts[i].y() > v[0]) v[0] = pts[i].y();
        if(pts[i].y() < v[1]) v[1] = pts[i].y();
    }

    maxU = ceil(u[0]), minU = floor(u[1]);
    maxV = ceil(v[0]), minV = floor(v[1]);
}

bool OctreeBasedVisualHullReconstructor::consistencyEvaluation(size_t minU, size_t maxU,
                                                               size_t minV, size_t maxV,
                                                               size_t imgIdx,
                                                               bool& hasIntersection, bool& isInside,
                                                               float adaptiveFactor = 1.0)
{
    size_t pixelCount = (maxV - minV + 1) * (maxU - minU + 1);
    size_t insideCount = 0;
    for(size_t v = minV; v <= maxV; v++)
    {
        for(size_t u = minU; u <= maxU; u++)
        {
            // if lie in the silhouette
            if(_silhouetteImages[imgIdx](u, v) == BinaryImage::VALUE_TRUE)
            {
                insideCount++;
            }
        }
    }

    float coverageRatio = (float)insideCount / (float) pixelCount;

    hasIntersection = (insideCount > 0);

    isInside = (insideCount == pixelCount);

    // factor is used for adaptive evaluation of consistency
    return ( coverageRatio < (_splittingThreshold + (1.0 - _splittingThreshold) * adaptiveFactor) );
}

list<Voxel> OctreeBasedVisualHullReconstructor::splitVoxel(const Voxel &v)
{
    list<Voxel> clist;

    float csx, csy, csz;
    csx = (v.xMax - v.xMin) * 0.5;
    csy = (v.yMax - v.yMin) * 0.5;
    csz = (v.zMax - v.zMin) * 0.5;
    for(int x=0;x<2;x++)
    {
        float xMin = v.xMin + x * csx;
        for(int y=0;y<2;y++)
        {
            float yMin = v.yMin + y * csy;
            for(int z=0;z<2;z++)
            {
                float zMin = v.zMin + z * csz;
                Voxel c;
                c.xMin = xMin, c.xMax = xMin + csx;
                c.yMin = yMin, c.yMax = yMin + csy;
                c.zMin = zMin, c.zMax = zMin + csz;
                c.r = c.g = c.b = c.a = 0;
                clist.push_back(c);
            }
        }
    }
    return clist;
}
