#ifndef ABSTRACTIMAGE_H
#define ABSTRACTIMAGE_H

#include "array2d.hpp"

#include <QSharedPointer>
#include <QImage>
#include <iostream>
#include <string>
#include <cassert>
#include <cstdlib>
using namespace std;

template <class PixelType>
class AbstractImage : public Array2D<PixelType>
{    
public:
    enum ImageType{
        RGBA,
        GRAYSCALE,
        BINARY,
        UNKNOWN
    };
    AbstractImage( );
    AbstractImage( ImageType t );
    AbstractImage( const string&, ImageType t );
    AbstractImage( int cols, int rows, int stride, ImageType t );
    AbstractImage( const AbstractImage& );
    AbstractImage( const PixelType* inputData, int cols, int rows, int stride, ImageType t );
    virtual ~AbstractImage();

    virtual AbstractImage<PixelType>& operator=(const AbstractImage<PixelType>&);

    const size_t& getColumn() const {return _width;}
    const size_t& getRow() const {return _height;}
    const size_t& getCols() const {return _width;}
    const size_t& getRows() const {return _height;}

    const size_t& getWidth() const {return _width;}
    const size_t& getHeight() const {return _height;}
    const size_t& width() const {return _width;}
    const size_t& height() const {return _height;}

    const size_t& stride() const {return _stride;}
    const size_t& getStride() const {return _stride;}

    const PixelType& value(size_t x, size_t y, size_t)const;
    PixelType& value(size_t x, size_t y, size_t);

    inline PixelType& operator() (size_t colIndex, size_t rowIndex, size_t compIndex = 0);
    inline const PixelType& operator() (size_t colIndex, size_t rowIndex, size_t compIndex = 0) const;

    inline PixelType& operator() (size_t index);
    inline const PixelType& operator() (size_t index) const;

    inline const PixelType* rawData() const {return _data;}

    virtual ImageType getType() const {return _type;}
    virtual QImage toQImage() = 0;
    
public:
    virtual bool loadImage(const string&) = 0;
    virtual bool saveImage(const string&) = 0;

protected:
    size_t _width;
    size_t _height;
    size_t _stride;
    PixelType* _data;
    ImageType _type;
    typedef PixelType* pData;
};

/*=========================================================================================
 * implementation of general image
 *=======================================================================================*/

template <class PixelType>
AbstractImage<PixelType>::AbstractImage():
    _width(0),
    _height(0),
    _stride(0),
    _data(pData(0)),
    _type(UNKNOWN)
{
}

template <class PixelType>
AbstractImage<PixelType>::AbstractImage( ImageType t ):
    _width(0),
    _height(0),
    _stride(0),
    _data(pData(0)),
    _type(t)
{
}

template <class PixelType>
AbstractImage<PixelType>::AbstractImage(const AbstractImage& img):
    _width(img._width),
    _height(img._height),
    _stride(img._stride),
    _type(img._type)
{
    size_t count = _width * _height * _stride;
    _data = new PixelType[count];
    memcpy(_data, img._data, sizeof(PixelType)*count);
}

template <class PixelType>
AbstractImage<PixelType>& AbstractImage<PixelType>::operator=(const AbstractImage<PixelType>& img)
{
    if(&img != this)
    {
        _width = img.getRows();
        _height = img.getHeight();
        _stride = img.getStride();
        _type = img.getType();
        _data = new PixelType[_width * _height * _stride];
        memcpy(_data, img.rawData(), sizeof(PixelType)*_width*_height*_stride);
        return *this;
    }
    else
    {
        return *this;
    }
}

template <class PixelType>
AbstractImage<PixelType>::AbstractImage(int cols, int rows, int stride = 1, ImageType t = UNKNOWN):
    _width(cols),
    _height(rows),
    _stride(stride),
    _data(new PixelType[_width * _height * _stride]),
    _type(t)
{
}

template <class PixelType>
AbstractImage<PixelType>::AbstractImage( const PixelType* inputData, int cols, int rows, int stride = 1, ImageType t = UNKNOWN ):
    _width(cols),
    _height(rows),
    _stride(stride),
    _type(t)
{    
    _data = new PixelType[_width * _height * _stride];
    memcpy(_data, inputData, sizeof(PixelType)*_width*_height*_stride);
}

template <class PixelType>
AbstractImage<PixelType>::~AbstractImage()
{
    if(_data != 0)
	delete[] _data;
}

template <class PixelType>
inline PixelType& AbstractImage<PixelType>::operator ()(size_t colIndex, size_t rowIndex, size_t compIndex)
{
    assert( colIndex < _width );
    assert( rowIndex < _height );
    assert( compIndex < _stride );
    return _data[(rowIndex * _width + colIndex) * _stride + compIndex];
}

template <class PixelType>
const PixelType& AbstractImage<PixelType>::operator ()(size_t colIndex, size_t rowIndex, size_t compIndex) const
{    
    assert( colIndex < _width );
    assert( rowIndex < _height );
    assert( compIndex < _stride );
    return _data[(rowIndex * _width + colIndex) * _stride + compIndex];
}

template <class PixelType>
PixelType& AbstractImage<PixelType>::operator ()(size_t index)
{
    assert( index < _width * _height * _stride );
    return _data[index];
}

template <class PixelType>
const PixelType& AbstractImage<PixelType>::operator ()(size_t index) const
{
    assert( index < _width * _height * _stride );
    return _data[index];
}

template <class PixelType>
const PixelType& AbstractImage<PixelType>::value(size_t x, size_t y, size_t c = 0) const
{
    if( x < _width
     && y < _height
     && c < _stride)
    {
        return _data[ (y * _width + x) * _stride + c ];
    }
    else
    {
        cerr<<"[[AbstractImage::value]] Index out of bound!";
        return _data[0];
    }
}

template <class PixelType>
PixelType& AbstractImage<PixelType>::value(size_t x, size_t y, size_t c = 0)
{
    if( x < _width
     && y < _height
     && c < _stride)
    {
        return _data[ (y * _width + x) * _stride + c ];
    }
    else
    {
        cerr<<"[[AbstractImage::value]] Index out of bound!";
        return _data[0];
    }
}

/******************************************************************************
 * definition of special images
******************************************************************************/


#endif // GENERALIMAGE_H
