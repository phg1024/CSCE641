#ifndef RECONSTRUCTOR_H
#define RECONSTRUCTOR_H

#include <string>
#include <fstream>
#include <sstream>

#include "rgbaimage.h"
#include "mathutil.hpp"

using namespace MathUtils;

// base class for 3d reconstructor

class Reconstructor : public QObject
{
    Q_OBJECT
public:
    Reconstructor();
    virtual ~Reconstructor();

    void setDescriptionFile( const string& desFilename ){ _desFilename = desFilename; }
    void setModelFile( const string& modelFilename ){ _modelFilename = modelFilename; }
    virtual void performReconstruction();
    virtual void outputModel(){}

signals:
    void sig_progress(double);

protected:
    bool loadProjectionMatrices();
    bool loadInputImages();


protected:
    size_t _filenameLength;
    string _filenameSuffix;
    string _name;
    string _desFilename;
    string _path;
    string _modelFilename;
    list<string> _inputImageFilename;
    string _cameraFilename;
    string _contourFilename;

protected:
    size_t _inputSize;
    // a set of input images
    RGBAImage* _inputImages;

    // a set of projection matrices
    DblMatrix* _projMat;

    // bounding box of the model
    double _xMin, _xMax, _yMin, _yMax, _zMin, _zMax;
};

#endif // RECTONSTRUCTOR_H
