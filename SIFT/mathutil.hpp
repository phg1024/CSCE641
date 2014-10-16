#ifndef MATH_UTIL_H
#define MATH_UTIL_H

#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdio>
using namespace std;

namespace MathUtils
{

template <typename T>
class Vector
{
    Vector():_length(0), _data(0){}
    Vector(size_t length):_length(length), _data(new T[length]){}
    Vector(const Vector& other):_length(other.length)
    {
	_data = new T[_length];
	memcpy(_data, other.rawData(), sizeof(T) * _length);
    }
    ~Vector()
    {
	if(_data != 0)
	    delete[] _data;
    }

    inline T& operator()(size_t idx)
    {
	if(idx < _length)
	    return _data[idx];
	else
	{
	    return _dummy;
	}
    }
	
    inline const T& operator()(size_t idx) const
    {
	if(idx < _length)
	    return _data[idx];
	else
	{
	    return _dummy;
	}
    }

    Vector& operator=(const Vector& other)
    {
	if(this == (&other))
	{
	    return (*this);
	}
	else
	{
	    if(_data != 0)
		delete[] _data;
	    _data = new T[other.length()];
	    memcpy(_data, other.rawData(), sizeof(T) * _length);
	}
    }
    
    const T operator*(const Vector& lhs) const
    {
	if(lhs.length() == _length)
	{
	    T sum = 0;
	    for(int i=0;i<_length;i++)
		sum += (_data[i] * lhs(i));
	}
	else
	{
	    return _dummy;
	}
    }
    
    const Vector operator*(const double& rhs) const
    {
	for(int i=0;i<_length;i++)
		_data[i] *= rhs;
    }
    
    const Vector operator+(const Vector& lhs) const
    {
	if(lhs.length() == _length)
	{
	    for(int i=0;i<_length;i++)
		_data[i] += lhs(i);
	}
	else
	    return Vector();
    }
    
    const Vector operator-(const Vector& lhs) const
    {
	return ((*this) + (-1) * lhs);
    }

    static Vector unit(size_t size)
    {
	Vector v(size);
	for(int i=0;i<size;i++)
	    v(i) = 1.0 / size;

	return v;
    }

    void normalize()
    {
	T _norm = norm();
	if(_norm > 0)
	{
	    for(int i=0;i<_length;i++)
		_data[i] /= _norm;
	}
    }

    T norm()
    {
	T sum = 0;
	for(int i=0;i<_length;i++)
		sum += (_data[i] * _data[i]);
	return sqrt(sum);
    }
    
    void print(const char* name = "")
    {
	printf("%s = \n", name);
	for(int i=0;i<_length;i++)
	    printf("%.6f\t", _data[i]);
    };

    inline const size_t& length() const {
        return _length;
    }
    const T* rawData() const {
        return _data;
    }

protected:
    void initialize()
    {
	memset(_data, 0, sizeof(T) * _length);
    }
    
private:
    size_t _length;
    T* _data;
    T _dummy;
};

template <typename T>
class Matrix
{
public:
    Matrix();
    Matrix(size_t rows, size_t cols);
    Matrix(const Matrix&);
    ~Matrix();

    inline T& operator()(size_t row, size_t col);
    inline const T& operator()(size_t row, size_t col) const;

    Matrix& operator=(const Matrix&);
    const Matrix operator*(const Matrix& lhs) const;
    const Matrix operator*(const double& rhs) const;
    const Matrix operator+(const Matrix& lhs) const;
    const Matrix operator-(const Matrix& lhs) const;

    Matrix inverted();
    Matrix transposed();
    static Matrix unit(size_t size);
    void print(const char* name = "");

    inline const size_t& rows() const {
        return _rows;
    }
    inline const size_t& cols() const {
        return _cols;
    }
    const T* rawData() const {
        return _data;
    }

protected:
    void initialize();

private:
    size_t _rows, _cols;
    T* _data;
    T _dummy;
};

template <typename T>
Matrix<T>::Matrix():
        _rows(0),
        _cols(0),
        _data(0)
{
}

template <typename T>
Matrix<T>::~Matrix()
{
    if (_data != 0)
        delete[] _data;
}

template <typename T>
Matrix<T>::Matrix(size_t rows, size_t cols):
        _rows(rows),
        _cols(cols),
        _data(new T[rows*cols])
{
}

template <typename T>
Matrix<T>::Matrix(const Matrix& other):
        _rows(other._rows),
        _cols(other._cols)
{
    _data = new T[_rows*_cols];
    memcpy(_data, other._data, sizeof(T) * _rows * _cols);
}

template <typename T>
T& Matrix<T>::operator()(size_t row, size_t col)
{
    if (row < _rows && col < _cols)
        return _data[row * _cols + col];
    else
    {
        return _dummy;
    }
}

template <typename T>
const T& Matrix<T>::operator()(size_t row, size_t col) const
{
    if (row < _rows && col < _cols)
        return _data[row * _cols + col];
    else
    {
        return _dummy;
    }
}

template <typename T>
Matrix<T>& Matrix<T>::operator=(const Matrix& other)
{
    if (this == &other)
    {
        return (*this);
    }
    else
    {
        _rows = other.rows();
        _cols = other.cols();
        if (_data != 0)
            delete[] _data;
        _data = new T[_rows * _cols];
        memcpy(_data, other.rawData(), sizeof(T) * _rows * _cols);
        return (*this);
    }
}

template <typename T>
const Matrix<T> Matrix<T>::operator*(const Matrix& op) const
{
    if (_cols == op.rows())
    {
        Matrix product(_rows, op.cols());
        for (size_t i=0;i<product.rows();i++)
            for (size_t j=0;j<product.cols();j++)
            {
                T sum = 0;
                for (size_t k=0;k<_cols;k++)
                    sum += (*this)(i, k) * op(k, j);
                product(i, j) = sum;
            }

        return product;
    }
    else
        return Matrix();
}

template <typename T>
const Matrix<T> Matrix<T>::operator*(const double& op) const
{
    Matrix product = (*this);
    for (size_t i=0;i<product.rows();i++)
        for (size_t j=0;j<product.cols();j++)
            product(i, j) = op * product(i, j);

    return product;
}

template <typename T>
const Matrix<T> Matrix<T>::operator+(const Matrix& op) const
{
    if ( rows() == op.rows()
            && cols() == op.cols() )
    {
        Matrix sum = op;
        for (size_t i=0;i<sum.rows();i++)
            for (size_t j=0;j<sum.cols();j++)
                sum(i, j) += op(i, j);

        return sum;
    }
    else
        return Matrix();
}

template <typename T>
const Matrix<T> Matrix<T>::operator-(const Matrix& op) const
{
    return ((*this) + (-1) * op);
}

template <typename T>
Matrix<T> Matrix<T>::inverted()
{
    if (_cols == _rows)
    {
        Matrix orig = (*this);
        Matrix inv = Matrix::unit(_cols);
        // perform gaussian elimination to achieve inverse

        // forward pass
        for (size_t i=0;i<_rows;i++)
        {
            T mainEle = orig(i, i);
            for (size_t j = i + 1;j < _rows;j++)
            {
                T curMain = orig(j, i);
                T factor = (-curMain/mainEle);
                for (size_t k=0;k<_cols;k++)
                {
                    orig(j, k) = factor * orig(i, k) + orig(j, k);
                    inv(j, k) = factor * inv(i, k) + inv(j, k);
                }
            }
            for (size_t k=0;k<_cols;k++)
            {
                orig(i, k) /= mainEle;
                inv(i, k) /= mainEle;
            }
        }

        // backward pass
        for (int i=_rows-1;i>=0;i--)
        {
            T mainEle = orig(i, i);
            for (int j = i - 1;j >= 0;j--)
            {
                T curMain = orig(j, i);
                T factor = (-curMain/mainEle);
                for (size_t k=0;k<_cols;k++)
                {
                    orig(j, k) += factor * orig(i, k);
                    inv(j, k) += factor * inv(i, k);
                }
            }
        }

        return inv;
    }
    else
        return Matrix();
}

template <typename T>
Matrix<T> Matrix<T>::transposed()
{
    Matrix t(_cols, _rows);
    for (int i=0;i<t.rows();i++)
        for (int j=0;j<t.cols();j++)
            t(i, j) = (*this)(j, i);

    return t;
}

template <typename T>
Matrix<T> Matrix<T>::unit(size_t size)
{
    Matrix t(size, size);
    t.initialize();

    for (size_t i=0;i<size;i++)
        t(i, i) = 1;

    return t;
}

template <typename T>
void Matrix<T>::initialize()
{
    memset(_data, 0, sizeof(T) * _rows * _cols);
}

template <typename T>
void Matrix<T>::print(const char* name)
{
    printf("%s\n", name);
    for (size_t i=0;i<_rows;i++)
    {
        for (size_t j=0;j<_cols;j++)
        {
            printf("%.6f\t", (*this)(i,j));
        }
        printf("\n");
    }
}

typedef Matrix<double> DblMatrix;

}
#endif
