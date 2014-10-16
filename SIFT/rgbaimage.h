#ifndef RGBAIMAGE_H
#define RGBAIMAGE_H

#include <cstdlib>
#include <string>
using namespace std;

typedef struct
{
    double r, g, b, a;
}RGBAPixel;

class RGBAImage
{
public:
    RGBAImage();
    RGBAImage(size_t w, size_t h);
    RGBAImage(size_t w, size_t h, double value);
    RGBAImage(double* rawData, size_t w, size_t h);
    RGBAImage(const RGBAImage&);
    RGBAImage(const string&);

    RGBAImage& operator=(const RGBAImage&);

    ~RGBAImage();

    bool loadImage(const string&);
    bool saveImage(const string&);

    const size_t& width() const {return _width;}
    const size_t& height() const {return _height;}
    const double* rawData() const {return _data;}

    RGBAPixel getPixel(size_t x, size_t y);
    void setPixel(size_t x, size_t y, const RGBAPixel& value);
    
    static const double INVALID_VALUE;
    static const double MAX_VALUE;
    static const double MIN_VALUE;

private:
    size_t _width, _height;
    double* _data;
};

#endif // RGBAIMAGE_H
