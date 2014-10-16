#include "depthmap.h"
#include "utility.hpp"

#include <QImage>
#include <QColor>

#include <cfloat>
#include <cstdlib>
#include <iostream>
using namespace std;

DepthMap::DepthMap():
    AbstractImage(GRAYSCALE)
{
}

DepthMap::DepthMap(size_t w, size_t h):
    AbstractImage(w, h, 1, GRAYSCALE)
{
}

DepthMap::DepthMap(size_t w, size_t h, DepthMapPixel value):
    AbstractImage(w, h, 1, GRAYSCALE)
{
    for (size_t i=0;i<_width*_height;i++)
    {
        size_t offset = i;
        _data[offset] = value;
    }
}

DepthMap::DepthMap(DepthMapPixel* rawData, size_t w, size_t h, bool isRGBA):
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
        memcpy(_data, rawData, sizeof(DepthMapPixel)*_width*_height);
    }
}


DepthMap::DepthMap(const DepthMap& a):
    AbstractImage(dynamic_cast<const AbstractImage&>(a))
{
}

DepthMap& DepthMap::operator=(const DepthMap& img)
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
        _data = new DepthMapPixel[_width * _height];
        memcpy(_data, img.rawData(), sizeof(DepthMapPixel)*_width*_height);
        return *this;
    }
}

DepthMap::DepthMap(const std::string& filename):
    AbstractImage(GRAYSCALE)
{
    loadImage(filename);
}

DepthMap::~DepthMap()
{
}

bool DepthMap::loadImage(const string& filename)
{
    if(filename.size() <= 0)
        return false;

    QImage img(filename.c_str());
    _width = img.width();
    _height = img.height();
    _data = new DepthMapPixel[_width * _height];
    for (size_t y=0;y<_height;y++)
    {
        size_t rowOffset = y * _width;
        for (size_t x=0;x<_width;x++)
        {
            QRgb p = img.pixel(x, y);
            int r, g, b, a;
            Utils::interpretPixel(p, r, g, b, a);
            size_t pixelIdx = (x + rowOffset);
            DepthMapPixel value = Utils::convertToGrayScaleValue(r, g, b);
            _data[pixelIdx] = value;
        }
    }
    return true;
}

bool DepthMap::saveImage(const string& filename)
{
    if(filename.size() <= 0)
        return false;

    QImage img(_width,_height, QImage::Format_ARGB32);
    for (size_t y=0;y<_height;y++)
    {
        size_t rowOffset = y * _width;
        for (size_t x=0;x<_width;x++)
        {
            size_t pixelIdx = (x + rowOffset);
            int r, g, b, a;
            DepthMapPixel value = _data[pixelIdx];

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

DepthMapPixel DepthMap::getPixel(size_t x, size_t y)
{
    size_t yIdx = y % _height, xIdx = x % _width;

    size_t offset = yIdx * _width + xIdx;
    return _data[offset];
}

void DepthMap::getNeighbor(size_t x, size_t y, size_t size, DepthMapPixel* pixels)
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

void DepthMap::setPixel(size_t x, size_t y, const DepthMapPixel& value)
{
    size_t offset = y * _width + x;
    _data[offset] = value;
}

QImage DepthMap::toQImage()
{
    QImage img(_width,_height, QImage::Format_ARGB32);

    DepthMapPixel _diffPix = _maxPix - _minPix;
    _diffPix = (_diffPix == 0)?FLT_MIN:_diffPix;

    for (size_t y=0;y<_height;y++)
    {
        size_t rowOffset = y * _width;
        for (size_t x=0;x<_width;x++)
        {
            size_t pixelIdx = (x + rowOffset);
            int r, g, b, a;
            DepthMapPixel value = _data[pixelIdx];

            // scale by min/max pixel
            value = (value - _minPix) / _diffPix * 255.0;
            Utils::clamp<DepthMapPixel>(0, 255, value);

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

void DepthMap::findMinMaxDepth()
{
    _maxPix = -FLT_MAX, _minPix = FLT_MAX;
    for (size_t y=0;y<_height;y++)
    {
        size_t rowOffset = y * _width;
        for (size_t x=0;x<_width;x++)
        {
            size_t pixelIdx = (x + rowOffset);
            DepthMapPixel value = _data[pixelIdx];
            if( value > _maxPix ) _maxPix = value;
            if( value < _minPix ) _minPix = value;
        }
    }
}
