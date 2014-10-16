#include "sift.h"

#include "grayscaleimage.h"
#include "rgbaimage.h"
#include "imageoperator.h"
#include "mathutil.hpp"
using namespace MathUtils;

#include <cmath>
#include <cfloat>
#include <vector>
#include <fstream>
#include <limits>
#include <algorithm>
#include <list>
using namespace std;

#include <QImage>
#include <QPainter>

#define TEST_GETNEIGHBOR 0
#define TEST_ROTATE_IMAGE 0
#define ORIENTATION_INTERP 1

QImage SiftOperator::process(const string& filename)
{
    infilename = filename;
    RGBAImage img(filename);

    inImg = img;

    // test if the image is valid
    int width = img.width(), height = img.height();
    if (width == 0 || height == 0)
    {
        cerr << "Invalid image!" << endl;
        return QImage();
    }

    int shortEdge = (width > height) ? width : height;

    //! calculate constants
    cutOffSize = 4;
    // scales = 3 is assigned at initialization
    dogNumberPerOctave = scales + 2;
    gaussianNumberPerOctave = dogNumberPerOctave + 1;

    downsampleFactor = 0.5;
    octaves = ceil((log(cutOffSize) - log(shortEdge)) / log(downsampleFactor));

    // maxEdgeCurvature = 10.0 is assigned at initialization
    edgeTestThreshold = pow((maxEdgeCurvature + 1.0), 2.0) / maxEdgeCurvature;

    // sigma0 = 1.6 is assigned at initialization
    sigmak = pow(2.0, 1.0 / (double)scales);

    extrema_edge_size = 4;
    // contrastThreshold = 0.05 is assigned at initialization

    //! initialize input image
    // convert to grayscale image
    GrayScaleImage grayImage = ImageOperator::grayscale_CPU(img);

#if TEST_GETNEIGHBOR
    // test getNeighbor
    int nSize = 127;
    double* neighbor = new double[nSize * nSize];
    grayImage.getNeighbor(128, 128, nSize, neighbor);
    GrayScaleImage nImg(neighbor, nSize, nSize);
    nImg.saveImage("neighbor.png");
    delete[] neighbor;
#endif

#if TEST_ROTATE_IMAGE
    // test getNeighbor
    int nSize = 127;
    double* neighbor = new double[nSize * nSize];
    grayImage.getNeighbor(128, 128, nSize, neighbor);
    int rSize = 89;
    double* rotatedImage = new double[rSize * rSize];
    rotate_image(neighbor, nSize, rotatedImage, rSize, -0.25 * PI);
    GrayScaleImage nImg(neighbor, nSize, nSize);
    nImg.saveImage("neighbor.png");
    GrayScaleImage rImg(rotatedImage, rSize, rSize);
    rImg.saveImage("rotated.png");
    delete[] neighbor;
    delete[] rotatedImage;
#endif

    // upsampling the image by a factor of 2
    GrayScaleImage enlargedImage =
            ImageOperator::bilinearSampling_CPU(grayImage, 2.0);

    // initial smooth
    GrayScaleImage initialImage =
            ImageOperator::gaussianFilter_bidirectional_CPU(enlargedImage, sqrt(sigma0 * sigma0 - initialSigma * initialSigma * 4));

    allocateResources();

    buildGaussianPyramid(initialImage);

    if(outputGSPYMD)
        outputGaussianPyramid();

    buildDifferenceOfGaussianPyrmaid();

    if(outputDOGPYMD)
        outputDifferenceOfGaussianPyramid();

    //! below are suspicious
    detectScaleSpaceExtrema();

    if(outputExtrema)
        outputRawExtremaImage();

    refineExtremaLocation();

    if(outputExtrema)
        outputExtremaImage();

    filterKeypoints();

    if(outputExtrema)
        outputEdgeEliminatedExtremaImage();

    calculateKeypointScale();

    assignKeypointOrientation();

    QImage keypointImageWithScales = outputKeypointImageWithScales();

    calculateFeatureVectors();

    sortFeatureVectorByScale();

    outputFeatureVectors();

    releaseResources();

    return keypointImageWithScales;
}

string SiftOperator::makeFilename(const string & rawname,
                                  const string & spec)
{
    stringstream ss;
    string fn;
    ss << rawname.substr(0,
                         rawname.
                         find(".")) << "_" << spec << ".png";
    ss >> fn;
    return fn;
}

string SiftOperator::makeFilename(const string & rawname,
                                  const string & spec, int level,
                                  int slice)
{
    stringstream ss;
    string fn;
    ss << rawname.substr(0,
                         rawname.
                         find(".")) << "_" << spec << "_l" << level << "_s"
       << slice << ".png";
    ss >> fn;
    return fn;
}

inline double SiftOperator::rotate_orientation(double inOrient, double centralOrient)
{
    double orient = inOrient - centralOrient;
    if (orient < (-PI))
        orient += 2.0 * PI;
    if (orient > PI)
	orient -= 2.0 * PI;
    return orient;
}

inline void SiftOperator::rotate_image(const double* origImage, int origImageSize, double* rotatedImage, int rotatedImageSize, double angle)
{
    double cosVal = cos(angle), sinVal = sin(angle);
    double origImageOffset = origImageSize / 2.0;
    double imgOffset = rotatedImageSize / 2.0;
    for (int i=0;i<rotatedImageSize;i++)
    {
        double yPos = i - imgOffset + 0.5;
        for (int j=0;j<rotatedImageSize;j++)
        {
            double xPos = j - imgOffset + 0.5;
            double rotatedX, rotatedY;
            rotatedX = xPos * cosVal - yPos * sinVal;
            rotatedY = yPos * cosVal + xPos * sinVal;

            rotatedX = rotatedX + origImageOffset;
            rotatedY = rotatedY + origImageOffset;

            // bilinearly sample image
            int leftX = floor(rotatedX);
            int rightX = leftX + 1;
            double xRatio = rotatedX - leftX;
            int upY = floor(rotatedY);
            int downY = upY + 1;
            double yRatio = rotatedY - upY;
            double upLeft, upRight, downLeft, downRight;
            upLeft = origImage[upY * origImageSize + leftX];
            upRight = origImage[upY * origImageSize + rightX];
            downLeft = origImage[downY * origImageSize + leftX];
            downRight = origImage[downY * origImageSize + rightX];
            rotatedImage[i * rotatedImageSize + j] = upLeft * (1.0 - xRatio) * (1.0 - yRatio)
                    + upRight * xRatio * (1.0 - yRatio)
                    + downLeft * (1.0 - xRatio) * yRatio
                    + downRight * xRatio * yRatio;
        }
    }
}

void SiftOperator::calculateGradientAndOrientation(const double* img, double* grad, double* orient, int size)
{
    for (int i=0;i<size;i++)
    {
        int y = i;
        int nextY = y + 1;
        if(nextY >= size) nextY -= size;
        int prevY = y - 1;
        if(prevY < 0) prevY += size;
        int rowOffset = y * size;
        int nextRowOffset = nextY * size;
        int prevRowOffset = prevY * size;
        for (int j=0;j<size;j++)
        {
            int x = j;
            int nextX = x + 1;
            if(nextX >= size) nextX -= size;
            int prevX = x - 1;
            if(prevX < 0) prevX += size;
            double diffX = (img[rowOffset + nextX] - img[rowOffset + prevX]) * 0.5;
            double diffY = (img[nextRowOffset + x] - img[prevRowOffset + x]) * 0.5;
            double gradValue = sqrt(diffX * diffX + diffY * diffY);
            grad[rowOffset + x] = gradValue;
            orient[rowOffset + x] = atan2(diffY, diffX);
        }
    }
}

void SiftOperator::sortFeatureVectorByScale()
{
    keypoints.sort(FeatureComp());
}

void SiftOperator::outputFeatureVectors()
{
    cout << "writing feature vectors ... " << endl;
    list<Feature>::iterator kit = keypoints.begin();
    fstream keyfile;
    stringstream keyss;
    keyss << infilename.substr(0, infilename.find(".")) << ".key";
    string keyfilename;
    keyss >> keyfilename;
    keyfile.open(keyfilename.c_str(), ios::out);
    const char SEPERATOR = ' ';
    const char ENDLINE = '\n';
    keyfile << keypoints.size() << SEPERATOR << feature_vector_length << ENDLINE;
    while (kit !=  keypoints.end())
    {
        Feature f = (*kit);
        keyfile << f._imgX / 2.0 << SEPERATOR
                << f._imgY / 2.0 << SEPERATOR
                << f._scale << SEPERATOR
                << f._orientation << SEPERATOR
                << ENDLINE;
        for (int i=0;i<feature_vector_length;i++)
            keyfile << f._signature[i] << SEPERATOR;
        keyfile << ENDLINE;
        kit ++;
    }
    keyfile.close();
}

void SiftOperator::calculateFeatureVectors()
{
    // total number of window regions
    const int feature_vector_histogram_size = 8;
    // number of region each row
    const int descriptor_row_number = 4,descriptor_col_number = 4;

    double*** descriptor = new double**[descriptor_row_number];
    for(int i=0;i<descriptor_row_number;i++)
    {
        descriptor[i] = new double*[descriptor_col_number];
        for(int j=0;j<descriptor_col_number;j++)
            descriptor[i][j] = new double[feature_vector_histogram_size];
    }

    const double descriptor_scale_factor = 3.0;

    cout << keypoints.size() << endl;

    list < Feature >::iterator kit = keypoints.begin();
    int count = 0;
    while ( kit != keypoints.end() )
    {
        // cout << count++ << endl;
        // initialize the descriptor
        for(int i=0;i<descriptor_row_number;i++)
            for(int j=0;j<descriptor_col_number;j++)
                for(int k=0;k<feature_vector_histogram_size;k++)
                    descriptor[i][j][k] = 0;
        // for each feature, use the histograms of its neighboring pixels as feature vector
        Feature& f = (*kit);
        double feature_orient = f._orientation;
        int descriptor_window_width = descriptor_scale_factor * f._octaveScale;
        int raw_image_window_radius = descriptor_window_width * descriptor_row_number
                * 0.5 * sqrt(2) + 0.5;

        double sigma = raw_image_window_radius;

        // used to align the pixel to the feature's orientation
        double cosVal, sinVal;
        cosVal = cos(-feature_orient), sinVal = sin(-feature_orient);

        // for each pixel in the local window, compute its contribution to the histogram
        for(int y = -raw_image_window_radius;y<=raw_image_window_radius;y++)
        {
            for(int x = -raw_image_window_radius;x<=raw_image_window_radius;x++)
            {
                double rotated_x = x * cosVal - y * sinVal;
                double rotated_y = x * sinVal + y * cosVal;

                double descriptor_x = rotated_x / descriptor_window_width;
                double descriptor_y = rotated_y / descriptor_window_width;

                double descriptor_row_idx = descriptor_x + 0.5 * descriptor_col_number - 0.5;
                double descriptor_col_idx = descriptor_y + 0.5 * descriptor_row_number - 0.5;

                // if the indices of the descriptor fall within the local region, accumulate
                // histogram
                if( descriptor_row_idx > -1.0 && descriptor_row_idx < descriptor_row_number
                        && descriptor_col_idx > -1.0 && descriptor_col_idx < descriptor_col_number)
                {
                    // calculate the gradient magnitude and orientation
                    double grad = calculateGradientMagnitude(f, x, y);
                    double orient = calculateOrientation(f, x, y);
                    orient = rotate_orientation(orient, feature_orient);

                    // transform orient to [0, 1]
                    orient = (orient + PI) * 0.5 / PI;
                    double orientIdx = orient * feature_vector_histogram_size;

                    double weight = exp( - (x * x + y * y) / (2.0 * sigma * sigma));
                    double entryVal = weight * grad;
                    // distribute the entry to all its neiboring bins

                    int dLeft, dUp, oLeft;
                    dLeft = floor(descriptor_col_idx);
                    dUp = floor(descriptor_row_idx);
                    oLeft = floor(orientIdx);
                    double ddx, ddy, ddo;
                    ddx = descriptor_col_idx - dLeft, ddy = descriptor_row_idx - dUp, ddo = orientIdx- oLeft;

                    double v;
                    for(int dx=0;dx<=1;dx++)
                    {
                        int xIdx = dLeft + dx;
                        if(xIdx >= 0 && xIdx < descriptor_col_number)
                        {
                            v = entryVal * (dx == 0)?(1.0 - ddx):ddx;
                            for(int dy=0;dy<=1;dy++)
                            {
                                int yIdx = dUp + dy;
                                if(yIdx >=0 && yIdx < descriptor_row_number)
                                {
                                    v *= (dy == 0)?(1.0 - ddy):ddy;
                                    for(int dh=0;dh<=1;dh++)
                                    {
                                        int hIdx = oLeft + dh;
                                        if(hIdx >=0 && hIdx < feature_vector_histogram_size)
                                        {
                                            v *= (dh == 0)?(1.0 - ddo):ddo;
                                            descriptor[xIdx][yIdx][hIdx] += v;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // copy the descriptor to the signature
        for(int i=0;i<descriptor_row_number;i++)
        {
            int rIdx = descriptor_row_number * i;
            for(int j=0;j<descriptor_col_number;j++)
            {
                int dShift = (rIdx + j) * feature_vector_histogram_size;
                for(int k=0;k<feature_vector_histogram_size;k++)
                {
                    //cout << dShift << endl;
                    f._signature[dShift + k] = descriptor[i][j][k];
                }
            }
        }

        // normalize the vector
        Utils::normalizeVector(f._signature, feature_vector_length);

        // clamp the feature vector
        const double feature_vector_threshold = 0.2;
        for(int i=0;i<feature_vector_length;i++)
            if(f._signature[i] > feature_vector_threshold)
                f._signature[i] = feature_vector_threshold;

        // normalize the vector
        Utils::normalizeVector(f._signature, feature_vector_length);

        kit++;
    }

    for(int i=0;i<descriptor_row_number;i++)
    {
        for(int j=0;j<descriptor_col_number;j++)
            delete[] descriptor[i][j];
        delete[] descriptor[i];
    }
    delete[] descriptor;
}

QImage SiftOperator::outputKeypointImageWithScales()
{
    cout << "generating output keypoint image with scales ... " <<endl;
    QImage featureImg(inImg.width(), inImg.height(), QImage::Format_ARGB32);
    for (int y = 0; y < featureImg.height(); y++) {
        for (int x = 0; x < featureImg.width(); x++) {
            // copy the raw image
            RGBAPixel p = inImg.getPixel(x, y);
            int r, g, b;
            r = p.r * 255.0;
            g = p.g * 255.0;
            b = p.b * 255.0;

            QRgb value = qRgb(r, g, b);
            featureImg.setPixel(x, y, value);
        }
    }

    cout << keypoints.size() << " keypoints in total..."<<endl;
    QPainter p(&featureImg);
    p.setRenderHints(QPainter::Antialiasing);
    list < Feature >::iterator feat_it = keypoints.begin();
    while ( feat_it != keypoints.end() ) {
        Feature f = (*feat_it);

        int centerX = f._imgX / 2.0;
        int centerY = f._imgY / 2.0;

        float scaleFactor = f._scale;

        int windowSize = scaleFactor * 5.0;

        float orient = (f._orientation) * 0.5 / PI * 360.0;

        p.setPen(QColor(0, 255, 0, 225));
        p.translate(centerX, centerY);
        p.rotate(orient);
#ifdef BOX_SYMBOL
        p.drawRect(- windowSize / 2, - windowSize / 2, windowSize, windowSize);
#endif
        p.drawLine(0, 0, windowSize / 2, 0);

        // tail
        p.setPen(QColor(255, 0, 0, 225));
        p.drawPoint(windowSize / 2, 0);

        // head
        p.setPen(QColor(0, 0, 255, 225));
        p.drawPoint(0, 0);
        p.rotate(-orient);
        p.translate(-centerX, -centerY);

        feat_it++;
    }

    featureImg.save(makeFilename(infilename, "siftout").c_str());

    return featureImg;
}

double SiftOperator::calculateOrientation(Feature& f, int xOffset, int yOffset)
{
    const int edgeTestWindowSize = 3;
    double edgeTestPixels[9];
    gaussians[f._octaveIdx][f._scaleIdx].getNeighbor(f._x + xOffset, f._y + yOffset, edgeTestWindowSize, edgeTestPixels);

    double dx, dy;

    dx = (edgeTestPixels[5] - edgeTestPixels[3]) * 0.5;
    dy = (edgeTestPixels[7] - edgeTestPixels[1]) * 0.5;

    return atan2(dy, dx);
}

double SiftOperator::calculateGradientMagnitude(Feature& f, int xOffset, int yOffset)
{
    const int edgeTestWindowSize = 3;
    double edgeTestPixels[9];
    gaussians[f._octaveIdx][f._scaleIdx].getNeighbor(f._x + xOffset, f._y + yOffset, edgeTestWindowSize, edgeTestPixels);

    double dx, dy;

    dx = (edgeTestPixels[5] - edgeTestPixels[3]) * 0.5;
    dy = (edgeTestPixels[7] - edgeTestPixels[1]) * 0.5;

    return sqrt(dx * dx + dy * dy);
}

void SiftOperator::assignKeypointOrientation()
{
    list<Feature>::iterator kit = keypoints.begin();
    const int histogramSmoothSteps = 2;
    const int histogramSize = 36;
    double* histogram = new double[histogramSize];
    double* smoothedHistogram = new double[histogramSize];
    const double orientationWindowFactor = 3.0;
    const double orientationSigmaFactor = 1.5;
    const double windowScaleFactor = orientationSigmaFactor * orientationWindowFactor;
    while ( kit != keypoints.end() )
    {
        Feature& f = (*kit);
        memset(histogram, 0, sizeof(double)*histogramSize);

        int windowSize = windowScaleFactor * f._octaveScale;
        double sigma = orientationSigmaFactor * f._octaveScale;

        // acumulate historgam
        for (int i=-windowSize;i<=windowSize;i++)
        {
            for (int j=-windowSize;j<=windowSize;j++)
            {
                double grad, orient;
                grad = calculateGradientMagnitude(f, i, j);
                orient = calculateOrientation(f, i, j);

                double weight = evaluateNormalizedGaussianValue<double, int>(0, 0, i, j, sigma);
                int binIdx = ( orient + PI ) / PI * 0.5 * histogramSize;

                if (binIdx == histogramSize)
                    binIdx--;
                histogram[binIdx] += weight * grad;
            }
        }

        //         Utils::printArray<double>(histogram, histogramSize);
        
        // smooth the histogram
        for (int s=0;s<histogramSmoothSteps;s++)
        {
            for (int i=1;i<histogramSize;i++)
            {
                int prev = i - 1, next = (i + 1) % histogramSize;
                if (prev < 0) prev += histogramSize;
                smoothedHistogram[i] = 0.25 * histogram[prev]
                        + 0.50 * histogram[i]
                        + 0.25 * histogram[next];
            }
            memcpy(histogram, smoothedHistogram, sizeof(double) * histogramSize);
        }

        //         Utils::printArray<double>(smoothedHistogram, histogramSize);
        
        // find the peak
        int maxOrientIdx = -1;
	double maxOrientVal = 0;
        for (int i=0;i<histogramSize;i++)
	    if(smoothedHistogram[i] > maxOrientVal)
	    {
		maxOrientIdx = i;
		maxOrientVal = smoothedHistogram[i];
	    }

#if ORIENTATION_INTERP
	// perform interpolation
	double interpolatedOrient;
	int leftIdx, rightIdx;
	leftIdx = maxOrientIdx - 1;
	if(leftIdx < 0) leftIdx += histogramSize;
	rightIdx = maxOrientIdx + 1;
	if(rightIdx >= histogramSize) rightIdx -= histogramSize;
	double orientOffset = (smoothedHistogram[rightIdx] - smoothedHistogram[leftIdx]) * 0.5
                / (2.0 * smoothedHistogram[maxOrientIdx] - smoothedHistogram[rightIdx] - smoothedHistogram[leftIdx]);
        // cout << orientOffset << endl;
	interpolatedOrient = maxOrientIdx + orientOffset;

        // 	cout << maxOrientIdx << ", " << interpolatedOrient << endl;

	f._orientation = (2.0 * PI) * (interpolatedOrient / histogramSize) - PI;
#else
        f._orientation = (2.0 * PI) * (maxOrientIdx / (double)histogramSize) - PI;
#endif

        ++kit;
    }

    delete[] histogram;
    delete[] smoothedHistogram;
}


void SiftOperator::calculateKeypointScale()
{
    list<Feature>::iterator kit = keypoints.begin();
    while ( kit != keypoints.end() )
    {
        Feature& f = (*kit);
        double scaleVal = f._scaleIdx + f._subScalePos;
        f._scale = sigma0 * pow(2.0, f._octaveIdx + scaleVal / scales);
        f._octaveScale = sigma0 * pow(2.0, scaleVal / scales);
        ++kit;
    }
}

double SiftOperator::calculateGradientMagnitude(SiftOperator::Feature& f)
{
    return calculateGradientMagnitude(f, 0, 0);
}

double SiftOperator::calculateCurvature(SiftOperator::Feature& f)
{
    const int edgeTestWindowSize = 3;
    double edgeTestPixels[9];
    dogs[f._octaveIdx][f._scaleIdx].getNeighbor(f._x, f._y, edgeTestWindowSize, edgeTestPixels);

    double dxx, dyy, dxy;

    double v_twice = edgeTestPixels[4] * 2.0;
    dxx = (edgeTestPixels[3] + edgeTestPixels[5] - v_twice);
    dyy = (edgeTestPixels[7] + edgeTestPixels[1] - v_twice);
    dxy = 0.25 * (edgeTestPixels[0] - edgeTestPixels[2] - edgeTestPixels[6] + edgeTestPixels[8]);

    double trace = dxx + dyy;
    double det = dxx * dyy - dxy * dxy;

    if (det <=0 )
        return DBL_MAX;
    else
        return (trace * trace / det);
}

void SiftOperator::filterKeypoints()
{
    cout << "filtering keypoints ..." << endl;
    // filter keypoints by curvature and gradient values;
    list<Feature> tmpKeypoints;
    tmpKeypoints.assign(keypoints.begin(), keypoints.end());
    list<Feature>::iterator kit = tmpKeypoints.begin();
    keypoints.clear();

    const double gradThreshold = 0.1;
    double maxGrad = 0;
    while ( kit != tmpKeypoints.end() )
    {
        Feature& f = (*kit);
        double gradMag = calculateGradientMagnitude(f);
        f._gradient = gradMag;
        if (gradMag > maxGrad) maxGrad = gradMag;
        ++ kit;
    }

    kit = tmpKeypoints.begin();
    while ( kit != tmpKeypoints.end() )
    {
        Feature& f = (*kit);
        double curvature = calculateCurvature(f);

        bool pass = true;
        pass &= (curvature < maxEdgeCurvature);
        pass &= (f._gradient >= gradThreshold * maxGrad);
	
        if (pass)
            keypoints.push_back(f);

        ++ kit;
    }

    cout << "keypoints after edge elimination: " << keypoints.size() << endl;
}

DblMatrix SiftOperator::calculateDerivative(int octaveIdx, int scaleIdx, int x, int y)
{
    DblMatrix D(3, 1);
    double dx, dy, ds;
    dx = ( dogs[octaveIdx][scaleIdx    ].getPixel(x - 1, y    )
           - dogs[octaveIdx][scaleIdx    ].getPixel(x + 1, y    )
           ) * 0.5;
    dy = ( dogs[octaveIdx][scaleIdx    ].getPixel(x    , y + 1)
           - dogs[octaveIdx][scaleIdx    ].getPixel(x    , y - 1)
           ) * 0.5;
    ds = ( dogs[octaveIdx][scaleIdx + 1].getPixel(x    , y    )
           - dogs[octaveIdx][scaleIdx - 1].getPixel(x    , y    )
           ) * 0.5;

    D(0, 0) = dx;
    D(1, 0) = dy;
    D(2, 0) = ds;

    return D;
}

DblMatrix SiftOperator::calculateHessian(int octaveIdx, int scaleIdex, int x, int y)
{
    DblMatrix H(3, 3);
    double pixel, dxx, dyy, dss, dxy, dxs, dys;
    pixel = dogs[octaveIdx][scaleIdex].getPixel(x, y);
    double pixel2 = 2.0 * pixel;
    dxx = dogs[octaveIdx][scaleIdex    ].getPixel(x + 1, y    )
            + dogs[octaveIdx][scaleIdex    ].getPixel(x - 1, y    )
            - pixel2;
    dyy = dogs[octaveIdx][scaleIdex    ].getPixel(x    , y + 1)
            + dogs[octaveIdx][scaleIdex    ].getPixel(x    , y - 1)
            - pixel2;
    dss = dogs[octaveIdx][scaleIdex + 1].getPixel(x    , y    )
            + dogs[octaveIdx][scaleIdex - 1].getPixel(x    , y    )
            - pixel2;
    dxs = (dogs[octaveIdx][scaleIdex + 1].getPixel(x + 1, y   )
           - dogs[octaveIdx][scaleIdex + 1].getPixel(x - 1, y   )
           - dogs[octaveIdx][scaleIdex - 1].getPixel(x + 1, y   )
           + dogs[octaveIdx][scaleIdex - 1].getPixel(x - 1, y   )
           ) * 0.25;
    dys = (dogs[octaveIdx][scaleIdex + 1].getPixel(x    , y + 1)
           - dogs[octaveIdx][scaleIdex + 1].getPixel(x    , y - 1)
           - dogs[octaveIdx][scaleIdex - 1].getPixel(x    , y + 1)
           + dogs[octaveIdx][scaleIdex - 1].getPixel(x    , y - 1)
           ) * 0.25;
    dxy = (dogs[octaveIdx][scaleIdex    ].getPixel(x + 1, y + 1)
           - dogs[octaveIdx][scaleIdex    ].getPixel(x + 1, y - 1)
           - dogs[octaveIdx][scaleIdex    ].getPixel(x - 1, y + 1)
           + dogs[octaveIdx][scaleIdex    ].getPixel(x - 1, y - 1)
           ) * 0.25;

    H(0, 0) = dxx, H(0, 1) = dxy, H(0, 2) = dxs;
    H(1, 0) = dxy, H(1, 1) = dyy, H(1, 2) = dys;
    H(2, 0) = dxs, H(2, 1) = dys, H(2, 2) = dss;

    return H;
}

bool SiftOperator::testContrast(Feature& f, double dx, double dy, double ds)
{
    DblMatrix derivative, X(3, 1), product;
    derivative = calculateDerivative(f._octaveIdx, f._scaleIdx, f._x, f._y);
    X(0,0) = dx, X(1,0) = dy, X(2, 0) = ds;
    product = derivative * X;
    double contrast = dogs[f._octaveIdx][f._scaleIdx].getPixel(f._x, f._y) + product(0, 0) * 0.5;

    if ( abs(contrast) < contrastThreshold / scales)
        return true;
    else
        return false;
}

bool SiftOperator::refineExtremum(Feature& f, double& dx, double& dy, double& ds)
{
    DblMatrix derivative, hessian, invHessian, X;
    derivative = calculateDerivative(f._octaveIdx,
                                     f._scaleIdx,
                                     f._x,
                                     f._y);
    //     derivative.print();
    hessian = calculateHessian(f._octaveIdx,
                               f._scaleIdx,
                               f._x,
                               f._y);

    //     hessian.print();
    invHessian = hessian.inverted();
    //     invHessian.print();

    X = invHessian * derivative;
    //    X.print();
    dx = X(0,0), dy = X(1,0), ds = X(2,0);

    // test X
    return (
                (abs(dx) < 0.5)
                && (abs(dy) < 0.5)
                && (abs(ds) < 0.5)
                );
}

void SiftOperator::refineExtremaLocation()
{
    cout << "refining scale space extrema location ... " << endl;

    const int MAX_ITER_NUMBER = 5;
    list<Feature> tempKeypoints;
    tempKeypoints.assign(keypoints.begin(), keypoints.end());
    list<Feature>::iterator kit = tempKeypoints.begin();
    keypoints.clear();
    while (kit!=tempKeypoints.end())
    {
        Feature& f = (*kit);
        int width = dogs[f._octaveIdx][f._scaleIdx].width();
        int height = dogs[f._octaveIdx][f._scaleIdx].height();
        // move the extrema to a more accurate location
        int iterCount = 0;
        bool isRejected = false;
        double dx, dy, ds;
        while ( iterCount < MAX_ITER_NUMBER )
        {
            //	    cout << "round " << iterCount << endl;
            bool stop = refineExtremum(f, dx, dy, ds);

            if (stop)
                break;

            f._x += round(dx);
            f._y += round(dy);
            f._scaleIdx += round(ds);

            if (/* test rejection condition */
                    f._scaleIdx < 1      ||
                    f._scaleIdx > scales ||
                    f._x < extrema_edge_size ||
                    f._y < extrema_edge_size ||
                    f._x >= width - extrema_edge_size ||
                    f._y >= height - extrema_edge_size
                    )
            {
                isRejected = true;
                // 		cout << "rejected by out of bound ... " << endl;
                break;
            }

            iterCount ++;
        }

        //! test contrast
        if (!isRejected)
	{
	    isRejected |= testContrast(f, dx, dy, ds);
            //	    if(isRejected)
            //		cout << "rejected by contrast ... " << endl;
	}

        if (!isRejected)
        {
            f._imgX = (f._x + dx) * pow(2.0, f._octaveIdx);
            f._imgY = (f._y + dy) * pow(2.0, f._octaveIdx);
	    if( (f._imgX >= 0 && f._imgX <= 2.0 * inImg.width())
                    && (f._imgY >= 0 && f._imgY <= 2.0 * inImg.height()))
	    {
		f._subScalePos = ds;
		keypoints.push_back(f);
	    }
        }

        ++ kit;
    }
}

void SiftOperator::detectScaleSpaceExtrema()
{
    cout << "dectecting scale space extrema ... " << endl;

    keypoints.clear();
    for (int i = 0; i < octaves; i++) {
        for (int j = 1; j<= scales; j++) {
            GrayScaleImage & dog = dogs[i][j];
            GrayScaleImage & prevDog = dogs[i][j-1];
            GrayScaleImage & nextDog = dogs[i][j+1];
            // iterate over all pixels
            int w = dog.width();
            int h = dog.height();
            for (int y = extrema_edge_size; y < h - extrema_edge_size; y++)
            {
                for (int x = extrema_edge_size; x < w - extrema_edge_size; x++) {
                    double pixels[9];
                    dog.getNeighbor(x, y, 3, pixels);

                    double pixVal = pixels[4];

                    bool isMaxima(true), isMinima(true);
                    for (int m = 0; m < 9; m++) {
                        if (m == 4)
                            continue;
                        isMinima &= (pixels[m] > pixVal);
                        isMaxima &= (pixels[m] < pixVal);
                    }

                    bool pass = ( isMaxima || isMinima );

                    if ( pass )
                    {
                        // check for next dog
                        double nextPixels[9];
                        nextDog.getNeighbor(x, y, 3, nextPixels);
			for (int m = 0; m < 9; m++) {
			    isMinima &= (nextPixels[m] > pixVal);
			    isMaxima &= (nextPixels[m] < pixVal);
			}

			pass = ( isMaxima || isMinima );

                        if( pass )
			{
			    // check for prev dog
			    double prevPixels[9];
			    prevDog.getNeighbor(x, y, 3, prevPixels);
			    for (int m = 0; m < 9; m++) {
				isMinima &= (prevPixels[m] > pixVal);
				isMaxima &= (prevPixels[m] < pixVal);
			    }

			    pass = ( isMaxima || isMinima );
			}
                    }

                    if ( pass ) {
                        // add it as a feature candiate
                        Feature f(x, y, i, j);
			f._imgX = x / (double) w * inImg.width() * 2;
			f._imgY = y / (double) h * inImg.height() * 2;
			f._scale = 0.0001;
			f._octaveScale = 0.0001;
                        keypoints.push_back(f);
                    }
                }
            }
        }
    }

    cout << keypoints.size() << " extrema detected!" << endl;
}

void SiftOperator::outputRawExtremaImage()
{
    cout << "raw keypoints number = " << keypoints.size() << endl;
    // visualize the extrema
    RGBAImage filteredExtremaImg = inImg;
    list < Feature >::iterator fit = keypoints.begin();
    while (fit != keypoints.end()) {
        Feature f = (*fit);
        int x = f._x;
        int y = f._y;
        x = (x / (double) dogs[f._octaveIdx][0].width()) * filteredExtremaImg.width();
        y = (y / (double) dogs[f._octaveIdx][0].height()) * filteredExtremaImg.height();

        RGBAPixel p;
        p.r = 1.0;
        p.g = p.b = 0.0;
        p.a = 1.0;
        filteredExtremaImg.setPixel(x, y, p);
        ++fit;
    }
    filteredExtremaImg.
            saveImage(makeFilename(infilename, "extrema"));
}

void SiftOperator::outputEdgeEliminatedExtremaImage()
{
    // visualize the extrema
    RGBAImage filteredExtremaImg = inImg;
    list < Feature >::iterator fit = keypoints.begin();
    while (fit != keypoints.end()) {
        Feature f = (*fit);
        int x = f._imgX / 2.0;	//! because of initial image enlargement
        int y = f._imgY / 2.0;
        RGBAPixel p;
        p.r = 1.0;
        p.g = p.b = 0.0;
        p.a = 1.0;
        filteredExtremaImg.setPixel(x, y, p);
        ++fit;
    }
    filteredExtremaImg.
            saveImage(makeFilename(infilename, "extrema_edge_eliminated"));
}

void SiftOperator::outputExtremaImage()
{
    cout << "filtered keypoints number = " << keypoints.size() << endl;
    // visualize the extrema
    RGBAImage filteredExtremaImg = inImg;
    list < Feature >::iterator fit = keypoints.begin();
    while (fit != keypoints.end()) {
        Feature f = (*fit);
        int x = f._imgX / 2.0;	//! because of initial image enlargement
        int y = f._imgY / 2.0;
        RGBAPixel p;
        p.r = 1.0;
        p.g = p.b = 0.0;
        p.a = 1.0;
        filteredExtremaImg.setPixel(x, y, p);
        ++fit;
    }
    filteredExtremaImg.
            saveImage(makeFilename(infilename, "extrema_filtered"));
}

void SiftOperator::outputDifferenceOfGaussianPyramid()
{
    for (int i=0;i<octaves;i++)
    {
        for (int j=0;j<dogNumberPerOctave;j++)
        {
            dogs[i][j].saveImage(makeFilename(infilename, "dog", i, j), true);
        }
    }
}

void SiftOperator::buildDifferenceOfGaussianPyrmaid()
{
    cout << "building difference of gaussians pyramid ... " << endl;

    for (int i=0;i<octaves;i++)
    {
        for (int j=0;j<dogNumberPerOctave;j++)
        {
            dogs[i][j] = ImageOperator::difference_CPU(gaussians[i][j + 1], gaussians[i][j]);
        }
    }
}

void SiftOperator::outputGaussianPyramid()
{
    for (int i=0;i<octaves;i++)
    {
        for (int j=0;j<gaussianNumberPerOctave;j++)
        {
            gaussians[i][j].saveImage(makeFilename(infilename, "gs", i, j));
        }
    }
}

void SiftOperator::buildGaussianPyramid(const GrayScaleImage& initImg)
{
    double* sigma = new double[gaussianNumberPerOctave];
    sigma[0] = sigma0;

    for (int i=1;i<gaussianNumberPerOctave;i++)
    {
        double sigmaPrev = pow(sigmak, i - 1) * sigma0;
        double sigmaSum = sigmaPrev * sigmak;
        sigma[i] = sqrt(sigmaSum * sigmaSum - sigmaPrev * sigmaPrev);
    }


    //     Utils::printArray<double>(sigma, gaussianNumberPerOctave);

    GrayScaleImage curImg = initImg;
    cout << "building gaussian pyramid ... " << endl;
    for (int i = 0; i < octaves; i++) {
        // initial smooth
        gaussians[i][0] = curImg;
        for (int j = 1; j < gaussianNumberPerOctave; j++) {
            gaussians[i][j] =
                    ImageOperator::
                    gaussianFilter_bidirectional_CPU(gaussians[i][j - 1],
                                                     sigma[j]);
        }
        curImg =
                ImageOperator::
                bilinearSampling_CPU(gaussians[i][gaussianNumberPerOctave - 3], downsampleFactor);
    }

    delete[] sigma;
}

void SiftOperator::allocateResources()
{
    gaussians = new GrayScaleImage*[octaves];
    dogs = new GrayScaleImage*[octaves];

    for (int i=0;i<octaves;i++)
    {
        gaussians[i] = new GrayScaleImage[gaussianNumberPerOctave];
        dogs[i] = new GrayScaleImage[dogNumberPerOctave];
    }
}

void SiftOperator::releaseResources()
{
    for (int i=0;i<octaves;i++)
    {
        delete[] gaussians[i];
        delete[] dogs[i];
    }

    delete[] gaussians;
    delete[] dogs;
}
