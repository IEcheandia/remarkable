#pragma once

#include <cmath>

namespace nsDxfReader
{

template <class C>
struct XY
{
    double x, y;

    XY()
    {
        x = 0;
        y = 0;
    }

    XY(double x, double y)
        : x(x)
        , y(y)
    {
    }

    bool operator==(C const& c) const
    {
        return x == c.x && y == c.y;
    }

    bool operator!=(C const& c) const
    {
        return x != c.x || y != c.y;
    }

    XY operator*=(double s)
    {
        x *= s;
        y *= s;
        return *this;
    }

    XY operator /= (double s)
	{
			x /= s;
			y /= s;
			return *this;
	}

	void Maximize(C const & c)
	{
		if (c.x > x)
			x = c.x;
		if (c.y > y)
			y = c.y;
	}

	void Minimize(C const & c)
	{
		if (c.x < x)
			x = c.x;
		if (c.y < y)
			y = c.y;
	}
};

struct Vec2 : public XY<Vec2>
{
    Vec2() = default;
    Vec2(double x, double y)
        : XY(x, y)
    {
    }
};

struct Point2 : public XY<Point2>
{
    Point2() = default;
    Point2(double x, double y)
        : XY(x, y)
    {
    }
};

struct Transform2
{
    Vec2 x{1, 0};
    Vec2 y{0, 1};
    Point2 base;

    inline static Transform2 Rotation(double a);
};

inline Vec2 operator + (Vec2 const & a, Vec2 const & b)
{
	return {a.x + b.x, a.y + b.y};
}

inline Point2 operator + (Point2 const & a, Vec2 const & b)
{
	return {a.x + b.x, a.y + b.y};
}

inline Point2 operator + (Vec2 const & a, Point2 const & b)
{
	return {a.x + b.x, a.y + b.y};
}

inline Vec2 operator - (Point2 const & a, Point2 const & b)
{
	return {a.x - b.x, a.y - b.y};
}

inline Vec2 operator - (Vec2 const & a, Vec2 const & b)
{
	return {a.x - b.x, a.y - b.y};
}

inline Point2 operator - (Point2 const & a, Vec2 const & b)
{
	return {a.x - b.x, a.y - b.y};
}

inline Vec2 operator+=(Vec2& a, Vec2 const& b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

inline Vec2 operator-=(Vec2& a, Vec2 const& b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

inline Point2 operator+=(Point2& a, Vec2 const& b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

inline Point2 operator-=(Point2& a, Vec2 const& b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

inline Vec2 operator * (Vec2 const & a, double s)
{
	return {a.x * s, a.y * s};
}

inline Vec2 operator * (double s, Vec2 const & a)
{
	return {a.x * s, a.y * s};
}

inline Point2 operator * (Point2 const & a, double s) // TODO: remove
{
	return {a.x * s, a.y * s};
}

inline Point2 operator * (double s, Point2 const & a) // TODO: remove
{
	return {a.x * s, a.y * s};
}

inline Vec2 operator / (Vec2 const & a, double s)
{
	return {a.x / s, a.y / s};
}

inline double SqLength(Vec2 const & v)
{
	return v.x * v.x + v.y * v.y;
}

inline double Length(Vec2 const & v)
{
	return std::sqrt(SqLength(v));
}

inline double Dot(Vec2 const& a, Vec2 const& b)
{
    return a.x * b.x + a.y * b.y;
}

inline Vec2 Ortho(Vec2 const& v)
{
    return {-v.y, v.x};
}

inline Vec2 Normalized(Vec2 const & v)
{
	return v / Length(v);
}

inline Point2 Midpoint(Point2 const & a, Point2 const & b)
{
    return {(a.x + b.x) * .5, (a.y + b.y) * .5};
}

inline double SqDistance(Point2 const& a, Point2 const& b)
{
    return SqLength(a - b);
}

inline double Distance(Point2 const & a, Point2 const & b)
{
	return Length(a - b);
}

inline Point2 operator * (Transform2 const & t, Point2 const & p)
{
	return t.base + p.x * t.x + p.y * t.y;
}

inline Vec2 operator * (Transform2 const & t, Vec2 const & v)
{
	return v.x * t.x + v.y * t.y;
}

inline Transform2 Transform2::Rotation(double a)
{
    Vec2 x{std::cos(a), std::sin(a)};
    return {x, Ortho(x), {0, 0}};
}

inline Transform2 operator*(Transform2 const& a, Transform2 const& b)
{
    return {a * b.x, a * b.y, a * b.base};
}

struct Vec3
{
	double x = 0, y = 0, z = 0;

	operator Vec2 () const
	{
		return { x, y };
	}
};

inline Vec3 operator / (Vec3 const& a, double s)
{
	return { a.x / s, a.y / s, a.z / s };
}

inline double SqLength(Vec3 const& v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline double Length(Vec3 const& v)
{
	return std::sqrt(SqLength(v));
}

inline Vec3 Normalized(Vec3 const& v)
{
	return v / Length(v);
}

inline Vec3 Cross(Vec3 const& a, Vec3 const& b)
{
	return Vec3{
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

} // namespace
