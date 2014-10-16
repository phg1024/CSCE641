#ifndef GRAYSCALEIMAGE_H
#define GRAYSCALEIMAGE_H

#include "abstractimage.hpp"

#include <cstdlib>
#include <string>
using namespace std;

typedef unsigned char GrayScalePixel;

class GrayScaleImage : public AbstractImage<GrayScalePixel>
{
public:
    GrayScaleImage();
    GrayScaleImage(size_t w, size_t h);
    GrayScaleImage(size_t w, size_t h, GrayScalePixel value);
    GrayScaleImage(GrayScalePixel* rawData, size_t w, size_t h, bool isRGBA = false);
    GrayScaleImage(const GrayScaleImage&);
    GrayScaleImage(const string&);

    ~GrayScaleImage();

    GrayScaleImage& operator=(const GrayScaleImage&);

    QImage toQImage();
    bool loadImage(const string&);
    bool saveImage(const string&, bool needShift = false);

    GrayScalePixel getPixel(size_t x, size_t y);
    void getNeighbor(size_t, size_t, size_t, GrayScalePixel*);
    void setPixel(size_t x, size_t y, const GrayScalePixel& value);

    static const GrayScalePixel MAX_VALUE;
    static const GrayScalePixel MIN_VALUE;
};

#endif // GRAYSCALEIMAGE_H
