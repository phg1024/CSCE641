#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "util_common.h"

#include <QTime>
#include <QColor>
#include <cmath>
#include <cstdlib>
#include <iostream>
using namespace std;

namespace Utils{

template <typename T>
inline T random(const T& min, const T& max)
{
    return (min + (max - min) * (rand() / (double)RAND_MAX));
}

//! utitliy functions
template <typename T1>
inline T1 linearInterpolate(const T1& left, const T1& right, double ratio)
{
    return (1.0 - ratio) * left + ratio * right;
}

template <typename T>
inline T bilinearInterpolate(const T& upLeft, const T& upRight,
                             const T& downLeft, const T& downRight,
                             double hRatio, double vRatio)
{
    return    upLeft * (1.0 - hRatio) * (1.0 - vRatio)
            + upRight * hRatio * (1.0 - vRatio)
            + downLeft * (1.0 - hRatio) * vRatio
            + downRight * hRatio * vRatio;
}

template <typename T>
inline const T& sigmoid(const T& value, double scale = 1.0)
{
    return 1.0 / (1.0 + exp(-scale * value));
}

template <typename T>
inline const T& clamp(const T& lower, const T& upper, T&value)
{
    if(value < lower) value = lower;
    if(value > upper) value = upper;
    return value;
}

template <typename T>
inline void transformNormalizedSphereToOrtho(const T& radius, const T& theta, const T& phi, T& x, T& y, T& z)
{
    double th = theta;
    double ph = phi;

    th = th * PI;
    ph = ph * 2.0 * PI;
    double sinTheta = sin(th);
    double rSinTheta = radius * sinTheta;
    z = rSinTheta * cos(ph);
    x = rSinTheta * sin(ph);
    y = radius * cos(th);
}

template <typename T>
inline void transformOrthoToNormalizedSphere(const T& x, const T& y, const T& z, T& radius, T& theta, T& phi)
{
    radius = sqrt(x * x + y * y + z * z);
    theta = acos(y / radius) / PI;
    if( z == 0.0 )
        phi = (x>0)?0.25:0.75;
    else
    {
        phi = atan(x / z) / PI;
        if(z > 0)
        {
            if(phi > 0)
                phi *= 0.5;
            else
                phi = phi * 0.5 + 1.0;
        }
        else
        {
            phi = phi * 0.5 + 0.5;
        }
    }
}

template <typename T>
class DistCompare
{
public:
    bool operator()(const T& lhs, const T& rhs)
    {
        return (lhs.second < rhs.second)?true:false;
    };
};

template <typename T1, typename T2>
inline T1 evaluateNormalizedGaussianValue(const T2& centerX, const T2& centerY, const T2& x, const T2& y, const T1& sigma)
{
    T1 value;

    // evaluate two dimensional gaussian distribution
    T1 twoSigmaSquare = 2.0 * sigma *sigma;

    T2 diffX = x - centerX;
    T2 diffY = y - centerY;
    value = 1.0 / (PI * twoSigmaSquare) * exp(-(diffX * diffX + diffY * diffY) / twoSigmaSquare);
    return value;
}

template <typename T>
inline T generateRandomString(size_t length)
{
    T result;
    const float digitThreshold = 0.75;
    for(size_t i=0; i<length; i++)
    {
        char c;
        float digitRatio = rand() / (float)RAND_MAX;
        float ratio = rand() / (float)RAND_MAX;
        if(digitRatio > digitThreshold)
        {
            c = ratio * '0' + (1.0 - ratio) * '9';
        }
        else
        {
            c = ratio * 'a' + (1.0 - ratio) * 'z';
        }
        result +=c;
    }
    return result;
}

template <typename T>
void printMatrix(const T& mat, int row = 0, int col = 0)
{
    for(int i=0;i<row;i++)
    {
        for(int j=0;j<col;j++)
        {
            cout<<mat(i, j)<<((j == col - 1)?"":", ");
        }
        cout<<endl;
    }
}

template <typename T>
void printMatrix(const T* mat, int row = 0, int col = 0)
{
    for(int i=0;i<row;i++)
    {
        for(int j=0;j<col;j++)
        {
            cout<<mat[i * col + j]<<((j == col - 1)?"":", ");
        }
        cout<<endl;
    }
}

template <typename T1, typename T2>
void copyMatrix(const T1& mat1, T2 mat2, int row = 0, int col = 0)
{
    for(int i=0;i<row;i++)
    {
        for(int j=0;j<col;j++)
        {
            mat2[i * col + j] = mat1(i, j);
        }
    }
}

template <typename T1, typename T2>
void copyMatrix(const T1* mat1, T2* mat2, int row = 0, int col = 0)
{
    for(int i=0;i<row;i++)
    {
        for(int j=0;j<col;j++)
        {
            mat2[i * col + j] = mat1[i * col + j];
        }
    }
}

template <typename T>
void transposeMatrix(T* mat1, int row = 0, int col = 0)
{
    for(int i=0;i<row;i++)
    {
        for(int j=0;j<col;j++)
        {
	    T tmp = mat1[j * col + i];
            mat1[j * col + i] = mat1[i * col + j];
	    mat1[i * col + j] = tmp;
        }
    }
}

inline void interpretPixel(const QRgb& pixel, int& r, int& g, int& b, int& a)
{
    r = qRed(pixel);
    g = qGreen(pixel);
    b = qBlue(pixel);
    a = qAlpha(pixel);
}

template <typename T>
inline T convertToGrayScaleValue(T r, T g, T b)
{
    return 0.2989 * r + 0.5870 * g + 0.1141 * b;
}

template <typename T>
inline void printArray(const T* array_, int size)
{
    cout << endl;
    for(int i=0;i<size;i++)
	cout<< array_[i] << " ";
    cout << endl;
}

template <typename T>
inline void normalizeVector(T* array_, size_t length)
{
    T sum = 0;
    for(size_t idx = 0; idx < length; idx++)
	sum += array_[idx];

    for(size_t idx = 0; idx < length; idx++)
	array_[idx] /= sum;
}
}
#endif // UTILITY_HPP
