#ifndef BINARYIMAGE_H
#define BINARYIMAGE_H

#include "abstractimage.hpp"
#include <QImage>

#include <cstdlib>
#include <string>
using namespace std;

typedef unsigned char BinaryPixel;

class BinaryImage : public AbstractImage<BinaryPixel>
{
public:
    BinaryImage();
    BinaryImage(size_t w, size_t h);
    BinaryImage(size_t w, size_t h, BinaryPixel value);
    BinaryImage(BinaryPixel* rawData, size_t w, size_t h);
    BinaryImage(const BinaryImage&);
    BinaryImage(const string&);

    ~BinaryImage();

    BinaryImage& operator=(const BinaryImage&);

    QImage toQImage();
    bool loadImage(const string&);
    bool saveImage(const string&);

    BinaryPixel getPixel(size_t x, size_t y);
    void getNeighbor(size_t, size_t, size_t, BinaryPixel*);
    void setPixel(size_t x, size_t y, const BinaryPixel& value);

    static const BinaryPixel VALUE_1;
    static const BinaryPixel VALUE_0;
    static const BinaryPixel VALUE_TRUE;
    static const BinaryPixel VALUE_FALSE;
    static const BinaryPixel THRESHOLD;

    static BinaryImage fromQImage(const QImage&);

};

#endif // BINARYIMAGE_H
