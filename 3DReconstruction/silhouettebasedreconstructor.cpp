#include "silhouettebasedreconstructor.h"

#define SILHOUETTE_RECONSTRUCTOR_DEBUG 0

using namespace GeometryUtils;

SilhouetteBasedReconstructor::SilhouetteBasedReconstructor():
    Reconstructor(),
    _silhouetteImages(0)
{
}

SilhouetteBasedReconstructor::~SilhouetteBasedReconstructor()
{
    if(_silhouetteImages!=0)
        delete[] _silhouetteImages;
}

void SilhouetteBasedReconstructor::performReconstruction()
{
    Reconstructor::performReconstruction();

    // create silhouette images
    if(!loadContours())
        return;

    if(!buildSilhouetteImages())
        return;
}

bool SilhouetteBasedReconstructor::loadContours()
{
    _contours = new list<DblPolygon>[_inputSize];
    string absolutePath = _desFilename.substr(0, _desFilename.find_last_of("/"));
    string contourFilePath = absolutePath + "/" + _path + "/" + _contourFilename;
    fstream contourFile;
    contourFile.open(contourFilePath.c_str(), ios::in);
    if(!contourFile.good())
        return false;

    size_t lineIdx = 0;
    while(!contourFile.eof() && lineIdx < _inputSize)
    {
        DblPolygon polygon;
        size_t pointCount;
        contourFile >> lineIdx >> pointCount;
#if SILHOUETTE_RECONSTRUCTOR_DEBUG
        cout << "Contour #" << lineIdx << ": " << pointCount << " points." << endl;
#endif
        if(lineIdx >= _inputSize)
            break;

        for(size_t j=0;j<pointCount;j++)
        {
            DblPoint2D p;
            contourFile >> p.x() >> p.y();
            polygon.points.push_back(p);
        }

        GeometryUtils::assignPolygonOrientation(polygon);

        _contours[lineIdx].push_back(polygon);
    }

    return true;
}

bool SilhouetteBasedReconstructor::buildSilhouetteImages()
{
    cout << "building silhouette images ..." << endl;
    // detect if contour images already exists, if so, simply load those images
    list<int> missingSilhouetteIndices;
    list<string> missingSilhouette;
    list<string>::iterator it = _inputImageFilename.begin();
    int imgIdx = 0;
    while(it!=_inputImageFilename.end())
    {
        string sfname = makeSilhouetteImageFilename((*it));
        ifstream f(sfname.c_str());
        if(!f)
        {
            missingSilhouette.push_back(sfname);            
            missingSilhouetteIndices.push_back(imgIdx);
        }

        _silhouetteImageFiles.push_back(sfname);
        ++it;
        ++imgIdx;
    }

    if(missingSilhouetteIndices.empty())
    {
        cout << "Silhouette images exist!" << endl;
    }
    else
    {
        // build silhouette images for missing ones
        list<int>::iterator mit = missingSilhouetteIndices.begin();
        list<string>::iterator nmit = missingSilhouette.begin();
        while(mit!=missingSilhouetteIndices.end())
        {
            size_t idx = (*mit);
            string filename = (*nmit);
            RGBAImage& rImg = _inputImages[idx];
            size_t w, h;
            w = rImg.width(), h = rImg.height();

            // paint the silhouette into an image, then store it
            QImage sImg(w, h, QImage::Format_ARGB32);
            QPainter p(&sImg);
            p.fillRect(0, 0, w, h, Qt::white);
            p.setPen(Qt::black);

            list<DblPolygon>& contour = _contours[idx];
            list<DblPolygon>::iterator cit = contour.begin();
            bool mainOrient = contour.front().isClockWise;
            while(cit!=contour.end())
            {
                // convert DblPolygon into QPolygon
                DblPolygon& c = (*cit);
                QPolygonF poly;
                list<DblPoint2D>::iterator pit = c.points.begin();
                while(pit!=c.points.end())
                {
                    poly.push_back(QPointF((*pit).x(), (*pit).y()));
                    ++pit;
                }

                QPainterPath path;
                path.addPolygon(poly);
                // render the polygon into the silhouette image
                if(c.isClockWise == mainOrient)
                    p.fillPath(path, Qt::black);
                else
                    p.fillPath(path, Qt::white);
                ++cit;
            }

            sImg.save(QString::fromStdString(filename));
            ++mit;
            ++nmit;
        }
    }

    loadSilhouetteImages();
    return true;
}

std::string SilhouetteBasedReconstructor::makeSilhouetteImageFilename(const std::string &filename)
{
    // remove suffix
    string imagefilename = filename.substr(0, filename.find_last_of('.'));
    return imagefilename + "_contour" + "." + _filenameSuffix;
}

bool SilhouetteBasedReconstructor::loadSilhouetteImages()
{
    cout << "loading silhouette images ..." << endl;
    _silhouetteImages = new BinaryImage[_inputSize];
    list<string>::iterator it = _silhouetteImageFiles.begin();
    int idx = 0;
    while(it!=_silhouetteImageFiles.end())
    {
        _silhouetteImages[idx] = BinaryImage((*it));
        if( _silhouetteImages[idx].width() == 0 ||
            _silhouetteImages[idx].height() == 0 )
        {
            cout << "Failed to load silouette image " << (*it) << endl;
            return false;
        }
#if SILHOUETTE_RECONSTRUCTOR_DEBUG
        else
        {
            cout << "Silouette image #" << idx << ": " << _silhouetteImages[idx].width() << "x" << _silhouetteImages[idx].height() << endl;
        }
#endif
        ++it;
        ++idx;
    }
    return true;
}


