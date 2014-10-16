#include "rgbaimage.h"
#include "utility.hpp"
#include <QImage>
#include <QColor>

RGBAImage::RGBAImage():
    AbstractImage(RGBA)
{
}

RGBAImage::RGBAImage(size_t w, size_t h):
    AbstractImage(w, h, 4, RGBA)
{
}

RGBAImage::RGBAImage(size_t w, size_t h, double value):
    AbstractImage(w, h, 4, RGBA)
{
    for(size_t i=0;i<_width*_height;i++)
    {
        size_t offset = _stride * i;
        _data[offset + 0] = value;
        _data[offset + 1] = value;
        _data[offset + 2] = value;
        _data[offset + 3] = 255.0;
    }
}

RGBAImage::RGBAImage(RGBAComponent* rawData, size_t w, size_t h):
    AbstractImage(rawData, w, h, 4, RGBA)
{
}


RGBAImage::RGBAImage(const RGBAImage& a):
    AbstractImage(dynamic_cast<const AbstractImage&>(a))
{
}

RGBAImage& RGBAImage::operator=(const RGBAImage& img)
{
    if( &img != this)
    {
        if(_data != 0)
            delete[] _data;

        _width = img.width();
        _height = img.height();
        _stride = img.stride();
        _data = new RGBAComponent[_width * _height * _stride];
        memcpy(_data, img.rawData(), sizeof(RGBAComponent)*_width*_height*_stride);
        return *this;
    }
    else
    {
        return (*this);
    }
}

RGBAImage::RGBAImage(const std::string& filename):
    AbstractImage( RGBA )
{
    loadImage(filename);
}

RGBAImage::~RGBAImage()
{
}

bool RGBAImage::loadImage(const string& filename)
{
    if(filename.size() <= 0) return false;

    QImage img(filename.c_str());
    _width = img.width();
    _height = img.height();
    _stride = 4;
    //cout << _width << "x" << _height<<endl;
    _data = new RGBAComponent[_width * _height * _stride];
    for(size_t y=0;y<_height;y++)
    {
	size_t rowOffset = y * _width;
	for(size_t x=0;x<_width;x++)
	{
	    QRgb p = img.pixel(x, y);
	    int r, g, b, a;
	    Utils::interpretPixel(p, r, g, b, a);
            size_t pixelIdx = (x + rowOffset) * _stride;
            _data[pixelIdx + 0] = (RGBAComponent)r;
            _data[pixelIdx + 1] = (RGBAComponent)g;
            _data[pixelIdx + 2] = (RGBAComponent)b;
            _data[pixelIdx + 3] = (RGBAComponent)a;
	}
    }
    return true;
}

bool RGBAImage::saveImage(const string& filename)
{
    if(filename.size() <= 0) return false;
    QImage img = toQImage();
    return img.save(filename.c_str());
}

RGBAPixel RGBAImage::getPixel(size_t x, size_t y)
{
    RGBAPixel pixel;
    size_t yIdx = y % _height, xIdx = x % _width;
    size_t offset = (yIdx * _width + xIdx) * _stride;
    pixel.r = _data[offset + 0];
    pixel.g = _data[offset + 1];
    pixel.b = _data[offset + 2];
    pixel.a = _data[offset + 3];

    return pixel;
}

void RGBAImage::setPixel(size_t x, size_t y, const RGBAPixel& value)
{
    size_t offset = (y * _width + x) * _stride;
    _data[offset + 0] = value.r;
    _data[offset + 1] = value.g;
    _data[offset + 2] = value.b;
    _data[offset + 3] = value.a;
}

QImage RGBAImage::toQImage()
{
    QImage img(_width, _height, QImage::Format_ARGB32);
    for(size_t y=0;y<_height;y++)
    {
        size_t rowOffset = y * _width;
        for(size_t x=0;x<_width;x++)
        {
            size_t pixelIdx = (x + rowOffset) * _stride;
            int r, g, b, a;
            r = _data[pixelIdx + 0] * 255.0;
            r = Utils::clamp(0, 255, r);
            g = _data[pixelIdx + 1] * 255.0;
            g = Utils::clamp(0, 255, g);
            b = _data[pixelIdx + 2] * 255.0;
            b = Utils::clamp(0, 255, b);
            a = _data[pixelIdx + 3] * 255.0;
            a = Utils::clamp(0, 255, a);
            QRgb p = qRgba(r, g, b, a);
            img.setPixel(x, y, p);
        }
    }
    return img;
}
