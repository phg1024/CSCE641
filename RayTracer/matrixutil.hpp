#ifndef MATRIXUTIL_HPP
#define MATRIXUTIL_HPP

#include "mathutil.hpp"

using namespace MathUtils;

namespace MatrixUtils
{

template <typename T>
Matrix<T> makeTranslationMatrix(T dx, T dy, T dz)
{
    Matrix<T> mat(4, 4);
    mat(0, 0) = 1;      mat(0, 1) = 0;      mat(0, 2) = 0;      mat(0, 3) = dx;
    mat(1, 0) = 0;      mat(1, 1) = 1;      mat(1, 2) = 0;      mat(1, 3) = dy;
    mat(2, 0) = 0;      mat(2, 1) = 0;      mat(2, 2) = 1;      mat(2, 3) = dz;
    mat(3, 0) = 0;      mat(3, 1) = 0;      mat(3, 2) = 0;      mat(3, 3) = 1;

    return mat;
}

template <typename T>
Matrix<T> makeXRotationMatrix( T r )
{
    Matrix<T> mat(4, 4);

    double sinR = sin(r);
    double cosR = cos(r);

    mat(0, 0) = 1;      mat(0, 1) = 0;      mat(0, 2) = 0;      mat(0, 3) = 0;
    mat(1, 0) = 0;      mat(1, 1) = cosR;   mat(1, 2) = -sinR;  mat(1, 3) = 0;
    mat(2, 0) = 0;      mat(2, 1) = sinR;   mat(2, 2) = cosR;   mat(2, 3) = 0;
    mat(3, 0) = 0;      mat(3, 1) = 0;      mat(3, 2) = 0;      mat(3, 3) = 1;

    return mat;
}

template <typename T>
Matrix<T> makeXRotationMatrix_deg( T deg )
{
    T rad = deg / 180.0 * MathUtils::PI;
    return makeXRotationMatrix( rad );
}

template <typename T>
Matrix<T> makeYRotationMatrix( T r )
{
    Matrix<T> mat(4, 4);

    double sinR = sin(r);
    double cosR = cos(r);

    mat(0, 0) = cosR;   mat(0, 1) = 0;      mat(0, 2) = sinR;   mat(0, 3) = 0;
    mat(1, 0) = 0;      mat(1, 1) = 1;      mat(1, 2) = 0;      mat(1, 3) = 0;
    mat(2, 0) = -sinR;  mat(2, 1) = 0;      mat(2, 2) = cosR;   mat(2, 3) = 0;
    mat(3, 0) = 0;      mat(3, 1) = 0;      mat(3, 2) = 0;      mat(3, 3) = 1;

    return mat;
}

template <typename T>
Matrix<T> makeYRotationMatrix_deg( T deg )
{
    T rad = deg / 180.0 * MathUtils::PI;
    return makeYRotationMatrix( rad );
}

template <typename T>
Matrix<T> makeZRotationMatrix( T r )
{
    Matrix<T> mat(4, 4);

    double sinR = sin(r);
    double cosR = cos(r);

    mat(0, 0) = cosR;   mat(0, 1) = -sinR;  mat(0, 2) = 0;      mat(0, 3) = 0;
    mat(1, 0) = sinR;   mat(1, 1) = cosR;   mat(1, 2) = 0;      mat(1, 3) = 0;
    mat(2, 0) = 0;      mat(2, 1) = 0;      mat(2, 2) = 1;      mat(2, 3) = 0;
    mat(3, 0) = 0;      mat(3, 1) = 0;      mat(3, 2) = 0;      mat(3, 3) = 1;

    return mat;
}

template <typename T>
Matrix<T> makeZRotationMatrix_deg( T deg )
{
    T rad = deg / 180.0 * MathUtils::PI;
    return makeZRotationMatrix( rad );
}

enum RotationOrder
{
    XYZ,
    XZY,
    YXZ,
    YZX,
    ZXY,
    ZYX
};

template <typename T>
Matrix<T> makeRotationMatrix(T rx, T ry, T rz, RotationOrder order)
{
    Matrix<T> Rx, Ry, Rz;
    Rx = makeXRotationMatrix(rx);
    Ry = makeYRotationMatrix(ry);
    Rz = makeZRotationMatrix(rz);

    switch(order)
    {
    default:
    case XYZ:
    {
        return Rx * Ry * Rz;
    }
    case XZY:
    {
        return Rx * Rz * Ry;
    }
    case YXZ:
    {
        return Ry * Rx * Rz;
    }
    case YZX:
    {
        return Ry * Rz * Rx;
    }
    case ZXY:
    {
        return Rz * Rx * Ry;
    }
    case ZYX:
    {
        return Rz * Ry * Rx;
    }
    }
}

template <typename T>
Matrix<T> makeRotationMatrix_deg(T degX, T degY, T degZ, RotationOrder order)
{
    T radX = degX / 180.0 * MathUtils::PI;
    T radY = degY / 180.0 * MathUtils::PI;
    T radZ = degZ / 180.0 * MathUtils::PI;

    return makeRotationMatrix( radX, radY, radZ, order );
}

}

#endif // MATRIXUTIL_HPP
