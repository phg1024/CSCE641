#ifndef SIFT_H
#define SIFT_H

#include <QImage>

#include "grayscaleimage.h"
#include "rgbaimage.h"
#include "mathutil.hpp"
using namespace MathUtils;

#include <cstdlib>
#include <string>
#include <vector>
#include <list>
#include <cstring>
using namespace std;

class SiftOperator
{
public:
    SiftOperator(bool verbose = false):
        outputGSPYMD(verbose),
        outputDOGPYMD(verbose),
        outputExtrema(verbose)
    {
        scales = 3;
        maxEdgeCurvature = 10.0;
        contrastThreshold = 0.05;
        sigma0 = 1.6;
    }

    ~SiftOperator(){}

    enum Parameters{
        SCALES,
        EDGE_THRESHOLD,
        MAGNITUDE_THRESHOLD,
        INITIAL_SIGMA
    };

    void setParameter(Parameters p, double val)
    {
        switch(p)
        {
        case SCALES:
        {
            scales = (int)val;
            break;
        }
        case EDGE_THRESHOLD:
        {
            maxEdgeCurvature = val;
            break;
        }
        case MAGNITUDE_THRESHOLD:
        {
            contrastThreshold = val;
            break;
        }
        case INITIAL_SIGMA:
        {
            sigma0 = val;
            break;
        }
        }
    }
    
    void setMode(char m)
    {
        switch(m)
        {
        case 'v':
        {
            outputGSPYMD = true;
            outputExtrema = true;
            outputDOGPYMD = true;
            break;
        }
        case 'V':
        {
            outputGSPYMD = false;
            outputExtrema = false;
            outputDOGPYMD = false;
            break;
        }
        case 'g':
        {
            outputGSPYMD = true;
            break;
        }
        case 'G':
        {
            outputGSPYMD = false;
            break;
        }
        case 'e':
        {
            outputExtrema = true;
            break;
        }
        case 'E':
        {
            outputExtrema = false;
            break;
        }
        case 'd':
        {
            outputDOGPYMD = true;
            break;
        }
        case 'D':
        {
            outputDOGPYMD = false;
            break;
        }
        }
    }

    //! sift operation for input image file
    QImage process(const string& filename);

private:
    class Feature;
    
protected:
    //! main components
    inline double* calculateSigmas(double, double);
    inline void buildGaussianPyramid(const GrayScaleImage&);
    inline void buildDifferenceOfGaussianPyrmaid();
    inline void detectScaleSpaceExtrema();
    inline void refineExtremaLocation();
    inline void filterKeypoints();
    inline void calculateKeypointScale();
    inline void assignKeypointOrientation();
    inline void calculateFeatureVectors();
    inline void sortFeatureVectorByScale();

    //! i/o helpers
    inline void outputGaussianPyramid();
    inline void outputDifferenceOfGaussianPyramid();
    inline void outputRawExtremaImage();
    inline void outputExtremaImage();
    inline void outputEdgeEliminatedExtremaImage();
    inline QImage outputKeypointImageWithScales();
    inline void outputFeatureVectors();

    string makeFilename(const string&, const string&, int, int);
    string makeFilename(const string&, const string&);

    //! computation helpers
    inline bool refineExtremum(Feature&, double&, double&, double&);
    inline DblMatrix calculateDerivative(int octaveIdx, int scaleIdx, int x, int y);
    inline DblMatrix calculateHessian(int octaveIdx, int scaleIdex, int x, int y);
    inline bool testContrast(Feature&, double, double, double);
    inline double calculateCurvature(Feature&);
    inline double calculateGradientMagnitude(Feature&);
    inline double calculateCurvature(Feature&, int xOffset, int yOffset);
    inline double calculateGradientMagnitude(Feature&, int xOffset, int yOffset);
    inline double calculateOrientation(Feature&, int xOffset, int yOffset);
    inline double rotate_orientation(double inOrient, double centralOrient);
    inline void rotate_image(const double*, int, double*, int, double);
    inline void calculateGradientAndOrientation(const double*, double*, double*, int);

    //! memory management
    inline void releaseResources();
    inline void allocateResources();

private:
    //! io options
    bool outputGSPYMD, outputDOGPYMD, outputExtrema;
    
    //! parameters
private:
    //! gaussian pyramid related
    int cutOffSize;	//! cut off size of gaussian pyramids, the minimum size of image in the pyramid
    int scales;		//! scales in each octave
    double downsampleFactor;
    int octaves;		//! total number of octaves
    //! octaves = ceil(log(cutOffSize) - log(shortEdgeOfImage)) / log(scaleFactor)
    int gaussianNumberPerOctave;
    int dogNumberPerOctave;	//! number of difference of gaussian in each octave
    double initialSigma;
    double sigma0;		//! standard deviation for initial gaussian smooth kernel
    double sigmak;
    
    //! edge elimination related
    double maxEdgeCurvature;
    double edgeTestThreshold;	//! threshold = (maxEdgeCurvature + 1.0)^2 / maxEdgeCurvature;

    int extrema_edge_size;
    double contrastThreshold;

    static const int feature_vector_length = 128;

private:
    class FeatureComp
    {
    public:
        bool operator()(const Feature& f1, const Feature& f2)
	{
	    return (f1._scale > f2._scale);
	}
    };

    class Feature
    {
    public:
        Feature():
            _orientation(-1),
            _gradient(-1),
            _scale(-1),
            _x(-1),
            _y(-1),
            _octaveIdx(-1),
            _scaleIdx(-1),
            _subScalePos(0),
            _octaveScale(0)
	{}

        Feature(int x, int y, int octave, int scale):
            _x(x),
            _y(y),
            _octaveIdx(octave),
            _scaleIdx(scale)
	{}
        Feature(const Feature& other):
            _imgX(other._imgX),
            _imgY(other._imgY),
            _orientation(other._orientation),
            _gradient(other._gradient),
            _scale(other._scale),
            _x(other._x),
            _y(other._y),
            _octaveIdx(other._octaveIdx),
            _scaleIdx(other._scaleIdx),
            _subScalePos(other._subScalePos),
            _octaveScale(other._octaveScale)
	{
	    memcpy(_signature, other._signature, sizeof(double)*128);
	}

    public:
	// feature location
	double _imgX, _imgY;	//! position in original image
	double _orientation;	//! orientation
	double _gradient;	//! gradient magnitude
	double _scale;		//! scale of the feature

	int _x, _y;		//! position at the gaussian blurred image
	int _octaveIdx;		//! octave index
	int _scaleIdx;		//! scale index
	double _subScalePos;	//! sub scale position
	double _octaveScale;	//! relative scale in the octave
        double _signature[128]; //! signiture vector
    };
    
private:
    string infilename;
    RGBAImage inImg;
    GrayScaleImage** gaussians;
    GrayScaleImage** dogs;
    list<Feature> keypoints;
};

#endif	//SIFT_H
