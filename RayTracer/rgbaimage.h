#ifndef RGBAIMAGE_H
#define RGBAIMAGE_H

#include "abstractimage.hpp"

#include <cstdlib>
#include <string>
using namespace std;

typedef double RGBAComponent;

typedef struct
{
    RGBAComponent r, g, b, a;
}RGBAPixel;

class RGBAImage : public AbstractImage<RGBAComponent>
{
public:
    RGBAImage();
    RGBAImage(size_t w, size_t h);
    RGBAImage(size_t w, size_t h, double value);
    RGBAImage(RGBAComponent* rawData, size_t w, size_t h);
    RGBAImage(const RGBAImage&);
    RGBAImage(const string&);

    RGBAImage& operator=(const RGBAImage&);

    virtual ~RGBAImage();

    QImage toQImage();
    bool loadImage(const string&);
    bool saveImage(const string&);

    RGBAPixel getPixel(size_t x, size_t y);
    void setPixel(size_t x, size_t y, const RGBAPixel& value);
};

#endif // RGBAIMAGE_H
