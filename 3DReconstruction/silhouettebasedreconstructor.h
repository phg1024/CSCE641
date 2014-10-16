#ifndef SILHOUETTEBASEDRECONSTRUCTOR_H
#define SILHOUETTEBASEDRECONSTRUCTOR_H

#include "reconstructor.h"
#include "binaryimage.h"
#include "geometryutils.hpp"

#include <QImage>
#include <QPainter>
#include <QPolygonF>

class SilhouetteBasedReconstructor : public Reconstructor
{
public:
    SilhouetteBasedReconstructor();
    virtual ~SilhouetteBasedReconstructor();
    virtual void performReconstruction();
    virtual void outputModel(){}

protected:
    bool loadContours();
    bool buildSilhouetteImages();
    bool loadSilhouetteImages();
    string makeSilhouetteImageFilename(const string& filename);

protected:
    // a set of binary images for silhouette images
    list<string> _silhouetteImageFiles;
    BinaryImage* _silhouetteImages;
    list<GeometryUtils::DblPolygon>* _contours;
};

#endif // SILHOUETTEBASEDRECONSTRUCTOR_H
