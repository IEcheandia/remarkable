/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Sascha Demirovic, BA, HS
 *  @date			2010
 *  @brief			algorithmic interface for class TPoint
 */


template <class T>
TPoint<T> :: TPoint ()
: x(T(0)), y(T(0))
{
}

template <class T>
TPoint<T> :: TPoint( T x, T y )
: x(x), y(y)
{
}


template <class T>
bool operator==( const TPoint<T>& p1, const TPoint<T>& p2 )
{
	return ( p1.x == p2.x && p1.y == p2.y );
}

template <class T>
bool operator!=( const TPoint<T>& p1, const TPoint<T>& p2 )
{
	return ! (p1 == p2);
}

template <class T>
std::ostream& operator<<( std::ostream& os, const TPoint<T>& p ) {
	os << "<x=" << p.x << ", y=" << p.y << ">";
	return os;
}

template <class T>
TPoint<T> operator +( const TPoint<T>& p1, const TPoint<T>& p2 )
{
	return TPoint<T>(p1.x+p2.x, p1.y+p2.y);
}

template <class T>
TPoint<T> operator -( const TPoint<T>& p1, const TPoint<T>& p2 )
{
	return TPoint<T>(p1.x-p2.x, p1.y-p2.y);
}

/// multiply a a point by a factor
template <typename T>
TPoint<T> operator *( const TPoint<T>& p1, T p2 )
{
	return TPoint<T>(p1.x*p2, p1.y*p2);
}

/// divide a a point by a factor
template <typename T>
TPoint<T> operator /( const TPoint<T>& p1, T p2 )
{
	assert(p2 != 0);
	return TPoint<T>(p1.x / p2, p1.y / p2);
}

template <class T>
std::istream& operator>>( std::istream& is, TPoint<T>& p ) {
	is.ignore(5, '='); is >> p.x;
	is.ignore(5, '='); is >> p.y;
	is.ignore(5, '>');
	return is;
}
/// Abstand Punkt - Punkt
template <class T>
double distance( const TPoint<T> &p1, const TPoint<T> &p2 )
{
	double dx = double( p1.x - p2.x ),
		   dy = double( p1.y - p2.y );

	return sqrt( dx*dx + dy*dy );
}
template <class T>
double distance2( const TPoint<T> &p1, const TPoint<T> &p2 )
{
	double dx = double( p1.x - p2.x ),
		   dy = double( p1.y - p2.y );

	return dx*dx + dy*dy;
}
