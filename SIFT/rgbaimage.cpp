#include "rgbaimage.h"
#include "utility.hpp"
#include <QImage>
#include <QColor>

const double RGBAImage::INVALID_VALUE = -1.0;
const double RGBAImage::MAX_VALUE = 1.0;
const double RGBAImage::MIN_VALUE = 1.0;

RGBAImage::RGBAImage():
_width(0),
_height(0),
_data(0)
{
}

RGBAImage::RGBAImage(size_t w, size_t h):
_width(w),
_height(h),
_data(new double[w * h * 4])
{
}

RGBAImage::RGBAImage(size_t w, size_t h, double value):
_width(w),
_height(h),
_data(new double[w * h * 4])
{
    for(size_t i=0;i<_width*_height;i++)
    {
	size_t offset = 4 * i;
        _data[offset + 0] = value;
        _data[offset + 1] = value;
        _data[offset + 2] = value;
        _data[offset + 3] = 255.0;
    }
}

RGBAImage::RGBAImage(double* rawData, size_t w, size_t h):
_width(w),
_height(h),
_data(new double[w * h * 4])
{
    memcpy(_data, rawData, sizeof(double)*_width*_height*4);
}


RGBAImage::RGBAImage(const RGBAImage& a):
_width(a._width),
_height(a._height),
_data(new double[_width * _height * 4])
{
    memcpy(_data, a._data, sizeof(double)*_width*_height*4);
}

RGBAImage& RGBAImage::operator=(const RGBAImage& img)
{
    if( &img == this)
    {
	return (*this);
    }
    else
    {
	if(_data != 0)
	    delete[] _data;

	_width = img.width();
	_height = img.height();
 	_data = new double[_width * _height * 4];
	memcpy(_data, img.rawData(), sizeof(double)*_width*_height*4);
	return *this;
    }
}

RGBAImage::RGBAImage(const std::string& filename):
_width(0),
_height(0),
_data(0)
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
    cout << _width << "x" << _height<<endl;
    _data = new double[_width * _height * 4];
    for(size_t y=0;y<_height;y++)
    {
	size_t rowOffset = y * _width;
	for(size_t x=0;x<_width;x++)
	{
	    QRgb p = img.pixel(x, y);
	    int r, g, b, a;
	    Utils::interpretPixel(p, r, g, b, a);
	    size_t pixelIdx = (x + rowOffset) * 4;
	    _data[pixelIdx + 0] = r / 255.0;
	    _data[pixelIdx + 1] = g / 255.0;
	    _data[pixelIdx + 2] = b / 255.0;
	    _data[pixelIdx + 3] = a / 255.0;
	}
    }
    return true;
}

bool RGBAImage::saveImage(const string& filename)
{
    if(filename.size() <= 0) return false;
    QImage img(_width,_height, QImage::Format_ARGB32);
    for(size_t y=0;y<_height;y++)
    {
	size_t rowOffset = y * _width;
	for(size_t x=0;x<_width;x++)
	{
	    size_t pixelIdx = (x + rowOffset) * 4;
	    int r, g, b, a;
	    r = _data[pixelIdx + 0] * 255.0;
	    g = _data[pixelIdx + 1] * 255.0;
	    b = _data[pixelIdx + 2] * 255.0;
	    a = _data[pixelIdx + 3] * 255.0;
	    QRgb p = qRgba(r, g, b, a);
	    img.setPixel(x, y, p);
	}
    }
    return img.save(filename.c_str());
}

RGBAPixel RGBAImage::getPixel(size_t x, size_t y)
{
    RGBAPixel pixel;
    size_t yIdx = y % _height, xIdx = x % _width;
    size_t offset = (yIdx * _width + xIdx) * 4;
    pixel.r = _data[offset + 0];
    pixel.g = _data[offset + 1];
    pixel.b = _data[offset + 2];
    pixel.a = _data[offset + 3];

    return pixel;
}

void RGBAImage::setPixel(size_t x, size_t y, const RGBAPixel& value)
{
    size_t offset = (y * _width + x) * 4;
    _data[offset + 0] = value.r;
    _data[offset + 1] = value.g;
    _data[offset + 2] = value.b;
    _data[offset + 3] = value.a;
}
