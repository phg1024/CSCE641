#ifndef ARRAY2D_H
#define ARRAY2D_H

#include <cstdlib>

template <class T>
class Array2D
{
public:
    Array2D(){};

    //! get column number of the array
    virtual const size_t& getColumn() const = 0;
    //! get row number of the array
    virtual const size_t& getRow() const = 0;

    //! get an element at (rowIndex, columnIndex), for both reading and writing
    virtual T& operator() (size_t rowIndex, size_t columnIndex, size_t channelIndex = 0) = 0;

    //! get an element at (rowIndex, columnIndex), for reading only
    virtual const T& operator() (size_t rowIndex, size_t columnIndex, size_t channelIndex = 0) const = 0;

    //! get an element at (index), for both reading and writing
    virtual T& operator() (size_t index) = 0;

    //! get an element at (index), for reading only
    virtual const T& operator() (size_t index) const = 0;

    //! destructor, the subclass should override it
    virtual ~Array2D() {};
};

#endif // ARRAY2D_H
