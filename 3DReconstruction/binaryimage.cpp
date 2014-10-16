#include "binaryimage.h"

#include "utility.hpp"

#include <QImage>
#include <QColor>

#include <cstdlib>
#include <iostream>
using namespace std;

const BinaryPixel BinaryImage::VALUE_0 = 0;
const BinaryPixel BinaryImage::VALUE_1 = 255;
const BinaryPixel BinaryImage::VALUE_TRUE = 0;
const BinaryPixel BinaryImage::VALUE_FALSE = 255;
const BinaryPixel BinaryImage::THRESHOLD = 128;

BinaryImage::BinaryImage():
    AbstractImage(BINARY)
{
}

BinaryImage::BinaryImage(size_t w, size_t h):
    AbstractImage(w, h, 1, BINARY)
{
}

BinaryImage::BinaryImage(size_t w, size_t h, BinaryPixel value):
    AbstractImage(w, h, 1, BINARY)
{
    for (size_t i=0;i<_width*_height;i++)
    {
        size_t offset = i;
        _data[offset] = value;
    }
}

BinaryImage::BinaryImage(BinaryPixel* rawData, size_t w, size_t h):
    AbstractImage(rawData, w, h, 1, BINARY)
{
}


BinaryImage::BinaryImage(const BinaryImage& a):
    AbstractImage(dynamic_cast<const AbstractImage&>(a))
{
}

BinaryImage& BinaryImage::operator=(const BinaryImage& img)
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
        _data = new BinaryPixel[_width * _height];
        memcpy(_data, img.rawData(), sizeof(BinaryPixel)*_width*_height);
        return *this;
    }
}

BinaryImage::BinaryImage(const std::string& filename):
    AbstractImage(BINARY)
{
    loadImage(filename);
}

BinaryImage::~BinaryImage()
{
}

bool BinaryImage::loadImage(const string& filename)
{
    if(filename.size() <= 0)
        return false;

    QImage img(filename.c_str());
    _width = img.width();
    _height = img.height();
    _stride = 1;
    _data = new BinaryPixel[_width * _height];
    for (size_t y=0;y<_height;y++)
    {
        size_t rowOffset = y * _width;
        for (size_t x=0;x<_width;x++)
        {
            QRgb p = img.pixel(x, y);
            int r, g, b, a;
            Utils::interpretPixel(p, r, g, b, a);
            size_t pixelIdx = (x + rowOffset);
            BinaryPixel value = Utils::convertToGrayScaleValue(r, g, b);
            _data[pixelIdx] = (value >= THRESHOLD)?VALUE_1:VALUE_0;
        }
    }
    return true;
}

bool BinaryImage::saveImage(const string& filename)
{
    if(filename.size() <= 0)
        return false;

    QImage img = toQImage();
    return img.save(filename.c_str());
}

BinaryPixel BinaryImage::getPixel(size_t x, size_t y)
{
    size_t yIdx = y % _height, xIdx = x % _width;

    size_t offset = yIdx * _width + xIdx;
    return _data[offset];
}

void BinaryImage::getNeighbor(size_t x, size_t y, size_t size, BinaryPixel* pixels)
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

void BinaryImage::setPixel(size_t x, size_t y, const BinaryPixel& value)
{
    size_t offset = y * _width + x;
    _data[offset] = value;
}

QImage BinaryImage::toQImage()
{
    QImage img(_width,_height, QImage::Format_ARGB32);
    for (size_t y=0;y<_height;y++)
    {
        size_t rowOffset = y * _width;
        for (size_t x=0;x<_width;x++)
        {
            size_t pixelIdx = (x + rowOffset);
            int r, g, b, a;
            BinaryPixel value = _data[pixelIdx];
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

