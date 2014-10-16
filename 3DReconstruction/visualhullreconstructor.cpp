#include "visualhullreconstructor.h"
#include "utility.hpp"

using namespace Utils;

#define OUT_PROJ_IMG 0

VisualHullReconstructor::VisualHullReconstructor():
    voxel_size(1.0),
    inside_threshold(0.5),
    vaModel(0)
{
}

VisualHullReconstructor::~VisualHullReconstructor()
{
    if(vaModel != 0)
        delete vaModel;
}

void VisualHullReconstructor::performReconstruction()
{
    SilhouetteBasedReconstructor::performReconstruction();

    CPUReconstruction();

    cout << "visual hull done." << endl;
}

void VisualHullReconstructor::outputModel()
{
    vaModel->write(_modelFilename);
}

void VisualHullReconstructor::makeCorners(DblPoint3D* pts, float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
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

void VisualHullReconstructor::projectCorners(const DblMatrix& mat,
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

void VisualHullReconstructor::approximateFootprints(GeometryUtils::DblPoint3D *pts,
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

bool VisualHullReconstructor::evaluateFootprint(size_t minU, size_t maxU, size_t minV, size_t maxV, size_t imgIdx, bool& isInside)
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

    isInside = (insideCount == pixelCount);

    return ( ((float)insideCount / (float) pixelCount) < inside_threshold );
}

void VisualHullReconstructor::CPUReconstruction()
{
    cout << "performing visual hull reconstruction ..." << endl;

    size_t xSize = abs(_xMax - _xMin) / voxel_size;
    size_t ySize = abs(_yMax - _yMin) / voxel_size;
    size_t zSize = abs(_zMax - _zMin) / voxel_size;

    cout << "volume size = "
         << xSize << "x"
         << ySize << "x"
         << zSize << endl;

    float scaleX, scaleY, scaleZ;
    size_t maxDim = (xSize > ySize)?xSize:ySize;
    size_t minDim = (xSize < ySize)?xSize:ySize;
    maxDim = (maxDim > zSize)?maxDim:zSize;
    minDim = (minDim < zSize)?minDim:zSize;
    scaleX = (float) xSize / (float) maxDim, scaleY = (float) ySize / (float) maxDim, scaleZ = (float) zSize / (float) maxDim;

    DblPoint3D scaleVector(abs(_xMax - _xMin), abs(_yMax - _yMin), abs(_zMax - _zMin));
    cout << "scale vector = " << scaleVector.x() << ", "
         << scaleVector.y() << ", "
         << scaleVector.z() << endl;
    DblPoint3D translateVector(min(_xMax, _xMin),
                               min(_yMax, _yMin),
                               min(_zMax, _zMin));
    cout << "translation vector = " << translateVector.x() << ", "
         << translateVector.y() << ", "
         << translateVector.z() << endl;

    vaModel = new VoxelArrayModel(scaleX, scaleY, scaleZ);

#if OUT_PROJ_IMG
    vector<QImage> projImages;
    for(size_t i=0;i<_inputSize;i++)
    {
        QImage img = _inputImages[i].toQImage();
        projImages.push_back(img);
    }
#endif

    // loop over every pixel and try to color consistent ones
    float stepX = 1.0 / xSize;
    float stepY = 1.0 / ySize;
    float stepZ = 1.0 / zSize;
    float minX, maxX, minY, maxY, minZ, maxZ;
    minX = -0.5, maxX = 0.5;
    minY = -0.5, maxY = 0.5;
    minZ = -0.5, maxZ = 0.5;

    size_t paintedCount = 0;

    double progress = 0;
    double progressStep = stepX * stepY * stepZ;

    //for(size_t z=0; z<_resolution; z++)
    // z from 1 to 0
    for(size_t z= zSize - 1; z>0; z--)
    {
        qApp->processEvents();
        // z range of voxel
        float z0, z1;
        z0 = minZ + (z - 0.5) * stepZ;
        z1 = minZ + (z + 0.5) * stepZ;

        //for(size_t y=0;y<_resolution;y++)
        for(size_t y= ySize - 1; y>0; y--)
        {
            // y range of voxel
            float y0, y1;
            y0 = minY + (y - 0.5) * stepY;
            y1 = minY + (y + 0.5) * stepY;

            for(size_t x=0;x<xSize;x++)
            {
                progress += progressStep;
                emit sig_progress(progress);
                // x range of voxel
                float x0, x1;
                x0 = minX + (x - 0.5) * stepX;
                x1 = minX + (x + 0.5) * stepX;

                //cout << "processing voxel @ " << x << ", " << y << ", " << z << endl;

                DblPoint3D corners[8];
                DblPoint3D footprints[8];

                makeCorners(corners, x0, x1, y0, y1, z0, z1);

                bool badVoxel = false;
                bool inSideVoxel = true;

                // project the voxel to image plane
                for(size_t i=0;i<_inputSize;i++)
                {
                    //cout << "image #" << i << endl;
                    projectCorners(_projMat[i], corners, footprints, scaleVector, translateVector, 8);
#if OUT_PROJ_IMG
                    QImage& img = projImages[i];
                    QPainter p(&img);
#endif

                    // use a rectangle footprint to approximate the real footprint
                    // (u, v) is image space coordinates
                    int minU, maxU, minV, maxV;

                    approximateFootprints(footprints, minU, maxU, minV, maxV, 8);
                    int w = _inputImages[i].width(), h = _inputImages[i].height();

                    // shift the coordinate to align to image center
                    //                    cout << minU << ", " << maxU << "\t"
                    //                         << minV << ", " << maxV << endl;
                    //                    getchar();
                    list<pair<size_t, size_t> > validPixels;

                    // not int image plane
                    if( maxU < 0 || minU >= w
                            || maxV < 0 || minV >= h )
                    {
                        badVoxel = true;
                        break;
                    }
                    else
                    {
                        // restrict the footprint to the image plane
                        minU = clamp<int>(0, w - 1, minU); maxU = clamp<int>(0, w - 1, maxU);
                        minV = clamp<int>(0, h - 1, minV); maxV = clamp<int>(0, h - 1, maxV);
                    }

#if OUT_PROJ_IMG
                    //                    cout << "filling rect " << minU << " -> " << maxU
                    //                         << ", "
                    //                         << minV << "->" << maxV << endl;
                    p.fillRect( minU, minV, maxU - minU, maxV - minV,
                                QColor( (0.5 * (x0 + x1) + 0.5) * 255.0,
                                        (0.5 * (y0 + y1) + 0.5) * 255.0,
                                        (0.5 * (z0 + z1) + 0.5) * 255.0,
                                        25)
                                );
#endif

                    size_t footprintSize = (maxV - minV + 1) * (maxU - minU + 1);
                    if(footprintSize > 0)
                    {
                        // calculate footprint stats

                        bool isInside;

                        // get valid pixels in the footprint
                        badVoxel |= evaluateFootprint(minU, maxU, minV, maxV, i, isInside);

                        inSideVoxel &= isInside;

                        if(badVoxel)
                            break;

                        size_t footprintPixelCount = validPixels.size();
                        if(footprintPixelCount > 0)
                        {
#if 0
                            p.fillRect( minU, minV, maxU - minU, maxV - minV,
                                        QColor( (0.5 * (x0 + x1) + 0.5) * 255.0,
                                                (0.5 * (y0 + y1) + 0.5) * 255.0,
                                                (0.5 * (z0 + z1) + 0.5) * 255.0,
                                                25)
                                        );
#endif
                        }
                    }
                    else
                    {
                        badVoxel = true;
                        break;
                    }
                }

                if( badVoxel )
                    continue;

                if( inSideVoxel )
                    continue;

                // calculate consistency over all images
                paintedCount++;


                Voxel v;
                v.xMin = x0, v.xMax = x1;
                v.yMin = y0, v.yMax = y1;
                v.zMin = z0, v.zMax = z1;
                v.r = 175, v.g = 175, v.b = 175, v.a = 255;
                vaModel->addVoxel(v);
            }
        }
    }
    cout << paintedCount << " voxels painted." << endl;

#if OUT_PROJ_IMG
    // output projection images
    for(size_t i=0;i<_inputSize;i++)
    {
        string pname;
        stringstream ss;
        ss << "projimg_" << i << ".png";
        ss >> pname;
        projImages[i].save(pname.c_str());
    }
#endif
}

void VisualHullReconstructor::GPUReconstruction()
{
    // initial set up
    size_t xSize = abs(_xMax - _xMin) / voxel_size;
    size_t ySize = abs(_yMax - _yMin) / voxel_size;
    size_t zSize = abs(_zMax - _zMin) / voxel_size;

    cout << "volume size = "
         << xSize << "x"
         << ySize << "x"
         << zSize << endl;

    float scaleX, scaleY, scaleZ;
    size_t maxDim = (xSize > ySize)?xSize:ySize;
    size_t minDim = (xSize < ySize)?xSize:ySize;
    maxDim = (maxDim > zSize)?maxDim:zSize;
    minDim = (minDim < zSize)?minDim:zSize;
    scaleX = (float) xSize / (float) maxDim, scaleY = (float) ySize / (float) maxDim, scaleZ = (float) zSize / (float) maxDim;

    DblPoint3D scaleVector(abs(_xMax - _xMin), abs(_yMax - _yMin), abs(_zMax - _zMin));
    cout << "scale vector = " << scaleVector.x() << ", "
         << scaleVector.y() << ", "
         << scaleVector.z() << endl;

    // create rendering target
    size_t imgW, imgH;
    imgW = _inputImages[0].width(), imgH = _inputImages[0].height();
    size_t totalVoxelNum = xSize * ySize * zSize;
    size_t imgSize = ceil(sqrt((double)totalVoxelNum));
    QGLPixelBuffer *vhpb = new QGLPixelBuffer(imgSize, imgSize);
    QGLFramebufferObject *vhfbo = new QGLFramebufferObject(imgSize, imgSize);
    // create shader
    QGLShaderProgram *vhshader = new QGLShaderProgram();
    vhshader->addShaderFromSourceFile(QGLShader::Fragment, "shaders/visualhull.glsl");
    vhshader->link();

    GLuint* sTex = new GLuint[_inputSize];
    // upload textures
    for(size_t i=0;i<_inputSize;i++)
        sTex[i] = vhpb->bindTexture(_silhouetteImages[i].toQImage());

    unsigned char* volume = new unsigned char[xSize * ySize * zSize * 4];
    glEnable(GL_TEXTURE_3D);
    GLuint volumeTex;
    glGenTextures(1, &volumeTex);
    glBindTexture(GL_TEXTURE_3D, volumeTex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, xSize, ySize, zSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, volume);
    glDisable(GL_TEXTURE_3D);

    // perform rendering to get visual hull
    vhpb->makeCurrent();
    vhfbo->bind();

    gluOrtho2D(0, 1, 0, 1);
    glMatrixMode(GL_PROJECTION);

    vhshader->bind();

    // set parameters
    // input size
    vhshader->setUniformValue("numImg", (int)_inputSize);

    // volume size infomation
    vhshader->setUniformValue("xSize", (int)xSize);
    vhshader->setUniformValue("xSize", (int)ySize);
    vhshader->setUniformValue("xSize", (int)zSize);

    // scaling information
    vhshader->setUniformValue("scaleVec", QVector3D(scaleVector.x(), scaleVector.y(), scaleVector.z()));

    // projection matrices
    QMatrix3x4 *projMat = new QMatrix3x4[_inputSize];
    size_t pmIdx = 0;
    for(size_t i=0;i<_inputSize;i++)
    {
        for(size_t r=0;r<3;r++)
            for(size_t c=0;c<4;c++)
            {
                projMat[pmIdx++](r, c) = _projMat[i](r, c);
            }
    }
    vhshader->setUniformValueArray("projMat", projMat, _inputSize);

    // binary image textures
    vhshader->setUniformValueArray( "silhouettes", sTex, _inputSize );

    // render for zSize times
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(0, 1);
    glVertex2f(1, 0);
    glVertex2f(1, 1);
    glEnd();

    vhshader->bind();
    vhfbo->release();

    QImage vhImg = vhfbo->toImage();

    // convert visual hull texture to voxel array data
    for(int y=0;y<vhImg.height();y++)
        for(int x=0;x<vhImg.width();x++)
        {
            // for each pixel: white = occupied, black = carved
        }

    // wrap up
    delete vhpb;
    delete vhfbo;
    delete vhshader;
    delete[] sTex;
    delete[] volume;
    delete[] projMat;
}
