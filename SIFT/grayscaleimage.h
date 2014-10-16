#ifndef ALPHAMASK_H
#define ALPHAMASK_H

#include <cstdlib>
#include <string>
using namespace std;

typedef double GrayScalePixel;

class GrayScaleImage
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

    bool loadImage(const string&);
    bool saveImage(const string&, bool needShift = false);

    const size_t& width() const {return _width;}
    const size_t& height() const {return _height;}
    const GrayScalePixel* rawData() const {return _data;}

    GrayScalePixel getPixel(size_t x, size_t y);
    void getNeighbor(size_t, size_t, size_t, GrayScalePixel*);
    void setPixel(size_t x, size_t y, const GrayScalePixel& value);

    static const GrayScalePixel INVALID_VALUE;
    static const GrayScalePixel MAX_VALUE;
    static const GrayScalePixel MIN_VALUE;

    private:
    size_t _width, _height;
    GrayScalePixel* _data;
};

#endif // ALPHAMASK_H
