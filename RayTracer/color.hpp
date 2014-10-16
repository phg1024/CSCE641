#ifndef COLOR_HPP
#define COLOR_HPP

#include "geometryutils.hpp"
using namespace GeometryUtils;

#include "utility.hpp"

#include "rgbaimage.h"

template <typename T>
class Color4 : public Point4D<T>
{
public:
    Color4(T r, T g, T b, T a = 1):
        Point4D<T>(r, g, b, a)
    {}

    Color4():
        Point4D<T>()
    {}

    Color4(const Color4& other):
        Point4D<T>(other.r(), other.g(), other.b(), other.a())
    {}

    Color4(const Point4D<T>& p):
        Point4D<T>(p)
    {}

    Color4& operator=(const Color4& other)
    {
        if( &other == this )
        {
            return *this;
        }
        else
        {
            this->r() = other.r();
            this->g() = other.g();
            this->b() = other.b();
            this->a() = other.a();

            return (*this);
        }
    }

    Color4 operator+(const Color4& op)
    {
        Color4 c = op;
        c.r() += this->r();
        c.g() += this->g();
        c.b() += this->b();
        c.a() += this->a();

        return c;
    }

    Color4 operator-(const Color4& op)
    {
        Color4 c = op;
        c.r() -= this->r();
        c.g() -= this->g();
        c.b() -= this->b();
        c.a() -= this->a();

        return c;
    }

    Color4 operator*(const double& op)
    {
        Color4 c = (*this);
        c.r() *= op;
        c.g() *= op;
        c.b() *= op;
        c.a() *= op;

        return c;
    }

    Color4 operator/(const double& op)
    {
        Color4 c = (*this);
        c.r() /= op;
        c.g() /= op;
        c.b() /= op;
        c.a() /= op;

        return c;
    }

    template <typename CT>
    friend Color4<CT> operator+(const Color4<CT>&, const Color4<CT>&);
    template <typename CT>
    friend Color4<CT> operator-(const Color4<CT>&, const Color4<CT>&);
    template <typename CT>
    friend Color4<CT> operator*(const Color4<CT>&, const double&);
    template <typename CT>
    friend Color4<CT> operator*(const double&, const Color4<CT>&);
    template <typename CT>
    friend Color4<CT> operator/(const Color4<CT>&, const Color4<CT>&);

    const T& r() const { return this->x(); }
    T& r() { return this->x(); }

    const T& g() const { return this->y(); }
    T& g() { return this->y(); }

    const T& b() const { return this->z(); }
    T& b() { return this->z(); }

    const T& a() const { return this->w(); }
    T& a() { return this->w(); }

    template <typename CT>
    friend ostream& operator << (ostream&, Color4<CT>&);

    Color4 clamp(double lower, double upper)
    {
        Color4 c;
        c.r() = Utils::clamp(lower, upper, r());
        c.g() = Utils::clamp(lower, upper, g());
        c.b() = Utils::clamp(lower, upper, b());
        c.a() = Utils::clamp(lower, upper, a());

        return c;
    }

    static Color4 blend(const Color4& c1, const Color4& c2)
    {
        Color4 c;
        c = c1 * (1.0 - c2.a()) + c2 * c2.a();
        c.a() = c1.a() * (1.0 - c2.a()) + c2.a();
        if( c.a() > 1.0 )
            c.a() = 1.0;
        return c;
    }

    RGBAPixel toRGBAPixel()
    {
        RGBAPixel pix;
        pix.r = r();
        pix.g = g();
        pix.b = b();
        pix.a = a();
        return pix;
    }
};

template <typename CT>
ostream& operator << (ostream& s, Color4<CT>& c)
{
    s << c.r() << ", "
      << c.g() << ", "
      << c.b() << ", "
      << c.a() << endl;

    return s;
}

template <typename CT>
Color4<CT> operator+(const Color4<CT>& lhs, const Color4<CT>& rhs)
{
    Color4<CT> c = lhs;
    return c + rhs;
}

template <typename CT>
Color4<CT> operator-(const Color4<CT>& lhs, const Color4<CT>& rhs)
{
    Color4<CT> c = lhs;
    return c - rhs;
}

template <typename CT>
Color4<CT> operator*(const Color4<CT>& lhs, const double& rhs)
{
    Color4<CT> c = lhs;
    return c * rhs;
}

template <typename CT>
Color4<CT> operator*(const double& lhs, const Color4<CT>& rhs)
{
    Color4<CT> c = rhs;
    return c * lhs;
}

template <typename CT>
Color4<CT> operator/(const Color4<CT>& lhs, const double& rhs)
{
    Color4<CT> c = lhs;
    return c / rhs;
}

typedef Color4<double> DblColor4;

#endif // COLOR_HPP
