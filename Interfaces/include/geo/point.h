#ifndef _POINT_H_
#define _POINT_H_
/**
 *  @file
 *  @author Sascha Demirovic, HS
 *  @date   18.07.06
 */

#include <iostream>
#include <cmath>
#include <vector>
#include <cassert>


namespace precitec
{
namespace geo2d
{

/// Einfache Punktklasse
/** Diese Klasse stellt einen Punkt mit ganzzahligen karthesischen Koordinaten
 *  x,y dar. Diese Punkte beziehen sich auf kein Koordinatensystem und sollten
 *  nur fuer lokal Berechnugen verwendet werden.
 */
template <class T>
class TPoint
{
public:

	/// Erzeugt einen Punkt mit den Koordinaten x=0, y=0
	TPoint();
	~TPoint()
	{
	}
	/// Erzeugt eine Punkt aus den vorgegebenen Koordinaten
	/** @param x x-Koordinate
	 *  @param y y-Koordinate
	 */
	TPoint( T x, T y );
	void swap(TPoint &p) { std::swap(x, p.x); std::swap(y, p.y); }
	T x, ///< x-Koordinate
	  y; ///< y-Koordinate
};



// -----------------------------------------------------------------------------
// ------------------------------ Funktionen -----------------------------------
// -----------------------------------------------------------------------------
/// Gleichheit (Komponentenweise == )
template <class T>
bool operator==( const TPoint<T>& p1, const TPoint<T>& p2 );

template <class T>
bool operator!=( const TPoint<T>& p1, const TPoint<T>& p2 );

/// Eingabe
template <class T>
std::istream& operator>>( std::istream& os, TPoint<T>& p );

/// Ausgabe
template <class T>
std::ostream& operator<<( std::ostream& os, const TPoint<T>& p );

/// Abstand Punkt - Punkt
template <class T>
double distance( const TPoint<T> &p1, const TPoint<T> &p2 );
template <class T>
double distance2( const TPoint<T> &p1, const TPoint<T> &p2 );

/// Punktaddition
template <class T>
TPoint<T> operator +( const TPoint<T>& p1, TPoint<T> const& p2 );

/// Punktsubtraction
template <class T>
TPoint<T> operator -( const TPoint<T>& p1, TPoint<T> const& p2 );

/// multiply a a point by a factor
template <typename T>
TPoint<T> operator *( const TPoint<T>& p1, T p2 );

/// divide a a point by a factor
template <typename T>
TPoint<T> operator /( const TPoint<T>& p1, T p2 );


// Template Implementierung einfuegen
/// @cond HIDDEN_SYMBOLS
#include "geo/point.hpp"
/// @endcond

// Typdefinitionen
typedef TPoint< int > 						Point;
typedef TPoint< double > 					DPoint;
typedef std::vector<Point> 					VecPoint;
typedef std::vector<DPoint> 				VecDPoint;

} // namespace geo2d
} // namespace precitec


#endif //_POINT_H_
