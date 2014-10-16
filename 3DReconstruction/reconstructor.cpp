#include "reconstructor.h"

#define RECONSTRUCTOR_DEBUG 0

Reconstructor::Reconstructor():
    _inputImages(0),
    _projMat(0)
{
}

Reconstructor::~Reconstructor()
{
    if(_inputImages != 0)
        delete[] _inputImages;
    if(_projMat != 0)
        delete[] _projMat;
}

void Reconstructor::performReconstruction()
{
    if(_desFilename.empty())
    {
        cout << "No description file specified." << endl;
        return;
    }

    // parse the description file
    fstream desFile;
    desFile.open(_desFilename.c_str(), ios::in);

    if(desFile.bad())
        return;

    string identifier;

    // input size
    desFile >> identifier;
    if(identifier != "NUMBER")
        return;
    desFile >> _inputSize;

    // name of model
    desFile >> identifier;
    if(identifier != "NAME")
        return;
    desFile >> _name;

    // input path
    desFile >> identifier;
    if(identifier != "PATH")
        return;
    desFile >> _path;

    // image file name length
    desFile >> identifier;
    if(identifier != "IMAGE_FILE_NAME_LENGTH")
        return;
    desFile >> _filenameLength;

    // image file name length
    desFile >> identifier;
    if(identifier != "IMAGE_FILE_SUFFIX")
        return;
    desFile >> _filenameSuffix;

    // camera file
    desFile >> identifier;
    if(identifier != "CAMERA")
        return;
    desFile >> _cameraFilename;

    // contour file
    desFile >> identifier;
    if(identifier != "CONTOUR")
        return;
    desFile >> _contourFilename;

    // model bounding box
    desFile >> identifier;
    if(identifier != "XRANGE")
        return;
    desFile >> _xMin >> _xMax;

    desFile >> identifier;
    if(identifier != "YRANGE")
        return;
    desFile >> _yMin >> _yMax;

    desFile >> identifier;
    if(identifier != "ZRANGE")
        return;
    desFile >> _zMin >> _zMax;

    desFile.close();

    if(!loadProjectionMatrices())
    {
        cout << "Failed to load projection matrices!" << endl;
        return;
    }

    if(!loadInputImages())
    {
        cout << "Failed to load input images!" << endl;
        return;
    }
}

bool Reconstructor::loadProjectionMatrices()
{
    cout << "loading projection matrices ... " << endl;
    // load input images and projection matrices
    string absolutePath = _desFilename.substr(0, _desFilename.find_last_of("/"));
    string cameraFilePath = absolutePath + "/" + _path + "/" + _cameraFilename;
    _projMat = new DblMatrix[_inputSize];
    fstream cameraFile;
    cameraFile.open(cameraFilePath.c_str(), ios::in);
    if(!cameraFile.good())
        return false;

    double sign[3];
    sign[0] = (_xMax > _xMin) ? 1.0 : -1.0;
    sign[1] = (_yMax > _yMin) ? 1.0 : -1.0;
    sign[2] = 1.0;

    for(size_t i=0;i<_inputSize;i++)
    {
        DblMatrix mat(3, 4);
        for(size_t r=0;r<3;r++)
        {
            for(size_t c=0;c<4;c++)
            {
                cameraFile >> mat(r, c);
                mat(r, c) *= sign[r];
            }
        }
        _projMat[i] = mat;


#if RECONSTRUCTOR_DEBUG
        string curMat;
        stringstream ss;
        ss << "Mat #" << i;
        ss >> curMat;
        mat.print(curMat.c_str());
#endif
    }

    return cameraFile.good();
}

bool Reconstructor::loadInputImages()
{
    string absolutePath = _desFilename.substr(0, _desFilename.find_last_of("/"));
    _inputImageFilename.clear();

    cout << "loading input images ... " << endl;
    _inputImages = new RGBAImage[_inputSize];
    if(_inputImages == 0)
        return false;

    for(size_t i=0;i<_inputSize;i++)
    {
        string curImgName;
        stringstream ss;
        int count = floor(log10((double)(i + 1e-6)));
        if(count < 0) count = 0;
        for(int j=_filenameLength - 1;j>count;j--)
            ss << '0';
        ss << i;
        ss << "." << _filenameSuffix;
        ss >> curImgName;

        // load input images and projection matrices
        string imageFilePath = absolutePath + "/" + _path + "/" + curImgName;

        _inputImageFilename.push_back(imageFilePath);

        RGBAImage curImg(imageFilePath);

        _inputImages[i] = curImg;

        if( _inputImages[i].width() == 0 ||
            _inputImages[i].height() == 0 )
        {
            cout << "Failed to load image " << curImgName << endl;
            return false;
        }
#if RECONSTRUCTOR_DEBUG
        else
        {
            cout << "Image #" << i << ": " << _inputImages[i].width() << "x" << _inputImages[i].height() << endl;
//            stringstream ss;
//            ss << "image" << i << ".png";
//            string iname;
//            ss >> iname;
//            _inputImages[i].saveImage(iname);
        }
#endif
    }

    return true;
}
