#include "voxelcoloringreconstructor.h"
#include "utility.hpp"

using namespace Utils;

#define OUT_PROJ_IMG 0
#define USE_SMALL_WINDOW 1
#define USE_SILHOUETTE 1

VoxelColoringReconstructor::VoxelColoringReconstructor():
    consistency_threshold(25.0),
    background_threshold(100.0),
    voxel_size(1.0),
    vaModel(0)
{
}

VoxelColoringReconstructor::~VoxelColoringReconstructor()
{
    if(vaModel != 0)
        delete vaModel;
}

void VoxelColoringReconstructor::performReconstruction()
{
    SilhouetteBasedReconstructor::performReconstruction();
    cout << "performing voxel coloring reconstruction ..." << endl;
    cout << "background threshold = " << background_threshold << endl;
    cout << "consistency threshold = " << consistency_threshold << endl;

    size_t xSize = abs(_xMax - _xMin) / voxel_size;
    size_t ySize = abs(_yMax - _yMin) / voxel_size;
    size_t zSize = abs(_zMax - _zMin) / voxel_size;

    cout << "volume size = "
         << xSize << "x"
         << ySize << "x"
         << zSize << endl;

    DblPoint3D scaleVector(abs(_xMax - _xMin), abs(_yMax - _yMin), abs(_zMax - _zMin));
    DblPoint3D translateVector(min(_xMax, _xMin),
                               min(_yMax, _yMin),
                               min(_zMax, _zMin));

    float scaleX, scaleY, scaleZ;
    size_t maxDim = (xSize > ySize)?xSize:ySize;
    size_t minDim = (xSize < ySize)?xSize:ySize;
    maxDim = (maxDim > zSize)?maxDim:zSize;
    minDim = (minDim < zSize)?minDim:zSize;
    scaleX = (float) xSize / (float) maxDim, scaleY = (float) ySize / (float) maxDim, scaleZ = (float) zSize / (float) maxDim;

    vaModel = new VoxelArrayModel(scaleX, scaleY, scaleZ);

    // image masks
    BinaryImage* masks = new BinaryImage[_inputSize];
    for(size_t i=0;i<_inputSize;i++)
    {
        // set all mask to be unmasked
        masks[i] = BinaryImage(_inputImages[i].width(), _inputImages[i].height(), BinaryImage::VALUE_FALSE);
    }

    // loop over every pixel and try to color consistent ones
    float stepX = 1.0 / xSize;
    float stepY = 1.0 / ySize;
    float stepZ = 1.0 / zSize;
    float minX, maxX, minY, maxY, minZ, maxZ;
    minX = -0.5, maxX = 0.5;
    minY = -0.5, maxY = 0.5;
    minZ = -0.5, maxZ = 0.5;

    size_t paintedCount = 0;
    size_t bgRejectCount = 0;
    size_t consRejectCount = 0;

    double progress = 0;
    double progressStep = stepX * stepY * stepZ;

    //for(size_t z=0; z<_resolution; z++)
    // z from 1 to 0
    for(int z= zSize - 1; z>=0; z--)
    {
        qApp->processEvents();

        // z range of voxel
        float z0, z1;
        z0 = minZ + (z - 0.5) * stepZ;
        z1 = minZ + (z + 0.5) * stepZ;

        //for(size_t y=0;y<_resolution;y++)
        for(int y= ySize - 1; y>=0; y--)
        {
            // y range of voxel
            float y0, y1;
            y0 = minY + (y - 0.5) * stepY;
            y1 = minY + (y + 0.5) * stepY;

            for(int x=0;x<xSize;x++)
            {
                progress += progressStep;
                emit sig_progress(progress);

                // x range of voxel
                float x0, x1;
                x0 = minX + (x - 0.5) * stepX;
                x1 = minX + (x + 0.5) * stepX;

                //cout << "processing voxel @ " << x << ", " << y << ", " << z << endl;

                size_t pixelCount(0);
                double red(0), green(0), blue(0);
                double sigma_r(0), sigma_g(0), sigma_b(0);

                list<PixelInfo> footprintPixels;

                size_t nonemptycount = 0;

                DblPoint3D corners[8];
                DblPoint3D footprints[8];

                makeCorners(corners, x0, x1, y0, y1, z0, z1);

                bool isValidVoxel = true;

                // project the voxel to image plane
                for(size_t i=0;i<_inputSize;i++)
                {
                    //cout << "image #" << i << endl;
                    projectCorners(_projMat[i], corners, footprints, scaleVector, translateVector, 8);

                    // use a rectangle footprint to approximate the real footprint
                    // (u, v) is image space coordinates
                    int minU, maxU, minV, maxV;
                    approximateFootprints(footprints, minU, maxU, minV, maxV, 8);
                    int w = _inputImages[i].width(), h = _inputImages[i].height();

                    // shift the coordinate to align to image center
                    //                    cout << minU << ", " << maxU << "\t"
                    //                         << minV << ", " << maxV << endl;
                    list<PixelInfo> pixels;

                    // not int image plane
                    if( maxU < 0 || minU >= w
                            || maxV < 0 || minV >= h )
                    {
                        isValidVoxel = false;
                        break;
                    }
                    else
                    {
                        // restrict the footprint to the image plane
                        minU = clamp<int>(0, w - 1, minU); maxU = clamp<int>(0, w - 1, maxU);
                        minV = clamp<int>(0, h - 1, minV); maxV = clamp<int>(0, h - 1, maxV);
                    }

                    size_t footprintSize = (maxV - minV - 1) * (maxU - minU - 1);
                    if(footprintSize > 0)
                    {
                        // calculate footprint stats

                        // get valid pixels in the footprint
                        getPixels(minU, maxU, minV, maxV, i, masks, pixels);
                    }
                    else
                    {
                        isValidVoxel = false;
                        break;
                    }

                    footprintPixels.insert(footprintPixels.end(), pixels.begin(), pixels.end());
                    nonemptycount += (pixels.size() > 0)?1:0;
                }

                //                if( !isValidVoxel )
                //                    continue;

                // calculate consistency over all images
                if(footprintPixels.size() > 0)
                {
                    consistencyTest(footprintPixels, red, green, blue, sigma_r, sigma_g, sigma_b);

                    double sigma = (sqrt(sigma_r) + sqrt(sigma_g) + sqrt(sigma_b)) / 3.0;
                    //cout << sigma << endl;
                    red = clamp<double>(0, 255, red);
                    green = clamp<double>(0, 255, green);
                    blue = clamp<double>(0, 255, blue);

                    if(sigma < consistency_threshold)
                    {
                        RGBAPixel p;
                        p.r = red, p.g = green, p.b = blue;
                        if( isBackgroundPixel(p) )
                        {
                            bgRejectCount++;
                            continue;
                        }

                        paintedCount++;

                        // color the voxel
                        Voxel v;
                        v.xMin = x0, v.xMax = x1;
                        v.yMin = y0, v.yMax = y1;
                        v.zMin = z0, v.zMax = z1;
                        v.r = (unsigned char)red, v.g = (unsigned char)green, v.b = (unsigned char)blue, v.a = 255;
                        vaModel->addVoxel(v);
                        markPixels(footprintPixels, masks);
                    }
                    else
                    {
                        consRejectCount++;
                    }
                }
            }
        }
    }
    cout << bgRejectCount << " voxels rejected by background test." << endl;
    cout << consRejectCount << " voxels rejected by consistency test." << endl;
    cout << paintedCount << " voxels painted." << endl;

    delete[] masks;
    cout << "voxel coloring done." << endl;
}

void VoxelColoringReconstructor::outputModel()
{
    vaModel->write(_modelFilename);
}

void VoxelColoringReconstructor::makeCorners(DblPoint3D* pts, float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
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

void VoxelColoringReconstructor::projectCorners(const DblMatrix& mat,
                                                DblPoint3D *pts,
                                                DblPoint3D *projPts,
                                                const DblPoint3D& scaleVec,
                                                const DblPoint3D& translateVec,
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
        p = p + translateVec;

        projPts[i] = DblPoint3D::fromVector(mat * DblPoint4D(p, 1.0).toVector());
        projPts[i].x() /= projPts[i].z();
        projPts[i].y() /= projPts[i].z();
    }
}

void VoxelColoringReconstructor::approximateFootprints(GeometryUtils::DblPoint3D *pts,
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

void VoxelColoringReconstructor::getPixels(const int &minU, const int &maxU,
                                           const int &minV, const int &maxV,
                                           size_t imgIdx,
                                           BinaryImage* masks,
                                           list<PixelInfo>& pixels)
{
#if USE_SMALL_WINDOW
    int midU = (minU + maxU) / 2, midV = (minV + maxV) / 2;
    const int windowSize = 3;
    // if not marked
    for(int i=-windowSize;i<=windowSize;i++)
        for(int j=-windowSize;j<=windowSize;j++)
        {
            int u, v;
            u = midU + j;
            v = midV + i;

            u = clamp<int>(0, masks[imgIdx].width() - 1, u);
            v = clamp<int>(0, masks[imgIdx].height() - 1, v);
#if USE_SILHOUETTE
            // if lie in the silhouette
            if(_silhouetteImages[imgIdx](u, v) == BinaryImage::VALUE_TRUE)
            {
#endif
                if( masks[imgIdx](u, v) == BinaryImage::VALUE_FALSE )
                {
                    PixelInfo p;
                    p.x = u, p.y = v, p.imgIdx = imgIdx;
                    pixels.push_back(p);
                }
#if USE_SILHOUETTE
            }
            else
            {
                PixelInfo p;
                p.x = -1, p.y = -1, p.imgIdx = -1;
                pixels.push_back(p);
            }
#endif
        }
#else
    for(int v = minV; v <= maxV; v++)
    {
        for(int u = minU; u <= maxU; u++)
        {
#if USE_SILHOUETTE
            // if lie in the silhouette
            if(_silhouetteImages[imgIdx](u, v) == BinaryImage::VALUE_TRUE)
            {
#endif
                // if not marked
                if( masks[imgIdx](u, v) == BinaryImage::VALUE_FALSE )
                {
                    PixelInfo p;
                    p.x = u, p.y = v, p.imgIdx = imgIdx;
                    pixels.push_back(p);
                }
#if USE_SILHOUETTE
            }
#endif
        }
    }
#endif
}

bool VoxelColoringReconstructor::isBackgroundPixel(const RGBAPixel &p)
{   
#if 0
    double maxChannel = p.r;
    if(p.g > maxChannel) maxChannel = p.g;
    if(p.b > maxChannel) maxChannel = p.b;
    if(maxChannel > background_threshold)
#else
    double avg = (p.r + p.g + p.b) / 3.0;
    if( avg > background_threshold )
#endif
        return true;
    else
        return false;
}

void VoxelColoringReconstructor::consistencyTest(const list<VoxelColoringReconstructor::PixelInfo> &pixels,
                                                 double &r, double &g, double &b,
                                                 double &sr, double &sg, double &sb)
{
    size_t backgroundPixelCount = 0;
    size_t validPixelCount = 0;
    list<PixelInfo>::const_iterator pit = pixels.begin();
    while( pit != pixels.end() )
    {

        const PixelInfo& pinfo = (*pit);
        if(pinfo.imgIdx < 0)
        {
            backgroundPixelCount++;
            ++pit;
            continue;
        }

        RGBAPixel p = _inputImages[pinfo.imgIdx].getPixel(pinfo.x, pinfo.y);

        r += p.r;
        g += p.g;
        b += p.b;

        validPixelCount++;
        ++pit;
    }

    // if more than half of pixels are background pixels
    const double bg_threshd = 0.85;
    if(backgroundPixelCount > bg_threshd * pixels.size())
    {
        sr = sg = sb = DBL_MAX;
        return;
    }

    r /= pixels.size();
    g /= pixels.size();
    b /= pixels.size();

    // standard deviation
    pit = pixels.begin();
    while( pit != pixels.end() )
    {
        const PixelInfo& pinfo = (*pit);
        if(pinfo.imgIdx < 0)
        {
            ++pit;
            sr += 255 * 255;
            sg += 255 * 255;
            sb += 255 * 255;
            continue;
        }

        RGBAPixel p = _inputImages[pinfo.imgIdx].getPixel(pinfo.x, pinfo.y);

        sr += (p.r - r) * (p.r - r);
        sg += (p.g - g) * (p.g - g);
        sb += (p.b - b) * (p.b - b);

        ++pit;
    }

    sr /= pixels.size();
    sg /= pixels.size();
    sb /= pixels.size();
}

void VoxelColoringReconstructor::markPixels(const list<VoxelColoringReconstructor::PixelInfo> &pixels, BinaryImage* masks)
{
    // mark pixels in the masks
    list<PixelInfo>::const_iterator pit = pixels.begin();
    while( pit != pixels.end() )
    {

        if((*pit).imgIdx < 0)
        {
            ++pit;
            continue;
        }

        const PixelInfo& pinfo = (*pit);
        masks[pinfo.imgIdx](pinfo.x, pinfo.y) = BinaryImage::VALUE_TRUE;

        ++pit;
    }
}
