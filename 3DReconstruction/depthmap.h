#ifndef DEPTHMAP_H
#define DEPTHMAP_H

#include "abstractimage.hpp"

#include <cstdlib>
#include <string>
using namespace std;

typedef float DepthMapPixel;

class DepthMap : public AbstractImage<DepthMapPixel>
{
public:
    DepthMap();
    DepthMap(size_t w, size_t h);
    DepthMap(size_t w, size_t h, DepthMapPixel value);
    DepthMap(DepthMapPixel* rawData, size_t w, size_t h, bool isRGBA = false);
    DepthMap(const DepthMap&);
    DepthMap(const string&);

    ~DepthMap();

    DepthMap& operator=(const DepthMap&);

    QImage toQImage();
    bool loadImage(const string&);
    bool saveImage(const string&);

    DepthMapPixel getPixel(size_t x, size_t y);
    void getNeighbor(size_t, size_t, size_t, DepthMapPixel*);
    void setPixel(size_t x, size_t y, const DepthMapPixel& value);

protected:
    void findMinMaxDepth();

private:
    DepthMapPixel _maxPix, _minPix;
};

#endif // DEPTHMAP_H
