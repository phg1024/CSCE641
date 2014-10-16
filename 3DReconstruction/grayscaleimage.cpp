#include "grayscaleimage.h"
#include "utility.hpp"

#include <QImage>
#include <QColor>

#include <cstdlib>
#include <iostream>
using namespace std;

const GrayScalePixel GrayScaleImage::MAX_VALUE = 255;
const GrayScalePixel GrayScaleImage::MIN_VALUE = 0;

GrayScaleImage::GrayScaleImage():
    AbstractImage(GRAYSCALE)
{
}

GrayScaleImage::GrayScaleImage(size_t w, size_t h):
    AbstractImage(w, h, 1, GRAYSCALE)
{
}

GrayScaleImage::GrayScaleImage(size_t w, size_t h, GrayScalePixel value):
    AbstractImage(w, h, 1, GRAYSCALE)
{
    for (size_t i=0;i<_width*_height;i++)
    {
        size_t offset = i;
        _data[offset] = value;
    }
}

GrayScaleImage::GrayScaleImage(GrayScalePixel* rawData, size_t w, size_t h, bool isRGBA):
    AbstractImage(w, h, 1, GRAYSCALE)
{
    if (isRGBA)
    {
        int size = w * h;
        for (int i=0;i<size;i++)
        {
            _data[i] = rawData[i * 4];
        }
    }
    else
    {
        memcpy(_data, rawData, sizeof(GrayScalePixel)*_width*_height);
    }
}


GrayScaleImage::GrayScaleImage(const GrayScaleImage& a):
    AbstractImage(dynamic_cast<const AbstractImage&>(a))
{
}

GrayScaleImage& GrayScaleImage::operator=(const GrayScaleImage& img)
{
    if ( &img == this)
    {
        return (*this);
    }
    else
    {
        if (_data != 0)
            delete[] _data;

        _width = img.width();
        _height = img.height();
        _stride = img.stride();
        _data = new GrayScalePixel[_width * _height];
        memcpy(_data, img.rawData(), sizeof(GrayScalePixel)*_width*_height);
        return *this;
    }
}

GrayScaleImage::GrayScaleImage(const std::string& filename):
    AbstractImage(GRAYSCALE)
{
    loadImage(filename);
}

GrayScaleImage::~GrayScaleImage()
{
}

bool GrayScaleImage::loadImage(const string& filename)
{
    if(filename.size() <= 0)
	return false;
    
    QImage img(filename.c_str());
    _width = img.width();
    _height = img.height();
    _data = new GrayScalePixel[_width * _height];
    for (size_t y=0;y<_height;y++)
    {
        size_t rowOffset = y * _width;
        for (size_t x=0;x<_width;x++)
        {
            QRgb p = img.pixel(x, y);
            int r, g, b, a;
            Utils::interpretPixel(p, r, g, b, a);
            size_t pixelIdx = (x + rowOffset);
            GrayScalePixel value = Utils::convertToGrayScaleValue(r, g, b);
            _data[pixelIdx] = value;
        }
    }
    return true;
}

bool GrayScaleImage::saveImage(const string& filename, bool needShift)
{
    if(filename.size() <= 0)
	return false;

    const double boostFactor = 2.0, shiftValue = 0.5;
    
    QImage img(_width,_height, QImage::Format_ARGB32);
    for (size_t y=0;y<_height;y++)
    {
        size_t rowOffset = y * _width;
        for (size_t x=0;x<_width;x++)
        {
            size_t pixelIdx = (x + rowOffset);
            int r, g, b, a;
            GrayScalePixel value = _data[pixelIdx];
            if(needShift)
                value = value * boostFactor + shiftValue;
            r = value;
            g = value;
            b = value;
            a = 255;
            QRgb p = qRgba(r, g, b, a);
            img.setPixel(x, y, p);
        }
    }
    return img.save(filename.c_str());    
}

GrayScalePixel GrayScaleImage::getPixel(size_t x, size_t y)
{
    size_t yIdx = y % _height, xIdx = x % _width;

    size_t offset = yIdx * _width + xIdx;
    return _data[offset];
}

void GrayScaleImage::getNeighbor(size_t x, size_t y, size_t size, GrayScalePixel* pixels)
{
    if ( (size % 2) == 0 )
    {
	int halfSize = size / 2;
        for (int k=-halfSize;k<halfSize;k++)
        {
            int pixelY = y + k;
            for (int l=-halfSize;l<=halfSize;l++)
            {
                int idx = (k + halfSize) * size + (l + halfSize);
                int pixelX = x + l;
                pixels[idx] = getPixel(pixelX, pixelY);
            }
        }
    }
    else
    {
        int halfSize = (size -1) / 2;
        for (int k=-halfSize;k<=halfSize;k++)
        {
            int pixelY = y + k;
            for (int l=-halfSize;l<=halfSize;l++)
            {
                int idx = (k + halfSize) * size + (l + halfSize);
                int pixelX = x + l;
                pixels[idx] = getPixel(pixelX, pixelY);
            }
        }
    }
}

void GrayScaleImage::setPixel(size_t x, size_t y, const GrayScalePixel& value)
{
    size_t offset = y * _width + x;
    _data[offset] = value;
}

QImage GrayScaleImage::toQImage()
{
    QImage img(_width,_height, QImage::Format_ARGB32);
    for (size_t y=0;y<_height;y++)
    {
        size_t rowOffset = y * _width;
        for (size_t x=0;x<_width;x++)
        {
            size_t pixelIdx = (x + rowOffset);
            int r, g, b, a;
            GrayScalePixel value = _data[pixelIdx];
            r = value;
            g = value;
            b = value;
            a = 255;
            QRgb p = qRgba(r, g, b, a);
            img.setPixel(x, y, p);
        }
    }
    return img;
}
