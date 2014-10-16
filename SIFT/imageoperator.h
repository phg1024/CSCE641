#ifndef IMAGEOPERATOR_H
#define IMAGEOPERATOR_H

#include "grayscaleimage.h"
#include "rgbaimage.h"
#include "utility.hpp"

#include <string>
#include <sstream>
#include <cmath>
#include <limits>
#include <cfloat>
using namespace std;

using namespace Utils;

namespace ImageOperator
{

RGBAImage gradient(GrayScaleImage&);
RGBAImage gradient_CPU(GrayScaleImage&);
GrayScaleImage gradientMagnitude_CPU(GrayScaleImage&);
GrayScaleImage grayscale(RGBAImage&);
GrayScaleImage grayscale_CPU(RGBAImage&);
GrayScaleImage gaussianFilter_CPU(GrayScaleImage&, const double&);
GrayScaleImage gaussianFilter_bidirectional_CPU(GrayScaleImage&, const double&);
RGBAImage extrema(GrayScaleImage&);
RGBAImage extrema_CPU(GrayScaleImage&);
GrayScaleImage difference_CPU(GrayScaleImage&, GrayScaleImage&);
GrayScaleImage bilinearSampling_CPU(GrayScaleImage&, double);

bool gradientMagnitude_CPU(GrayScaleImage&, GrayScaleImage&);
bool gradientMagnitudeAndOrientation_CPU(GrayScaleImage&, GrayScaleImage&, GrayScaleImage&);
bool gradientMagnitudeAndOrientation_CPU(GrayScaleImage&, GrayScaleImage&, GrayScaleImage&, GrayScaleImage&, GrayScaleImage&);
bool gradientMagnitudeAndOrientation_CPU(GrayScaleImage&, GrayScaleImage&, GrayScaleImage&, float& gradMax);
bool gradientMagnitudeAndOrientation_CPU(GrayScaleImage&, GrayScaleImage&, GrayScaleImage&, GrayScaleImage&, GrayScaleImage&, float& gradMax);

}
#endif
