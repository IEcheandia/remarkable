#ifndef _RECT_H_
#define _RECT_H_
/**
 * @file
 * @author Wolfgang Reichl
 * @date
 */

#include <iostream>
#include "geo/range.h"
#include "geo/point.h"
#include "geo/size.h"

#include <InterfacesManifest.h>

namespace precitec
{
namespace geo2d
{
	/// Rechteckklasse
	/** Stellt ein Rechteck durch zwei Intervalle dar. Der Zugriff in doppel
	 *  Schleifen ist dadurch intuitiever und effizienter.
	 */
	class INTERFACES_API Rect {
	public:
		/// Nullrechteck
		Rect() {}
		/// Rechteck aus zwei Intervallen
		Rect( const Range& x, const Range& y ): x_(x), y_(y) {}

		/// 0-Punkt-Rechteck mit Groesse Size=(dx, dy)
		Rect( const Size& s) : x_( 0, s.width-1 ), y_( 0, s.height-1 ) {
		}

		/// Rechteck aus Punkt und Abmessung
		Rect( const Point& p, const Size& s )
		: x_( p.x, p.x + s.width-1 ), y_( p.y, p.y + s.height-1 ) {
		}

		/// Rechteck aus zwei Punkten (beide inklusiv
		Rect( const Point& p1, const Point& p2 )
		: x_(Range::validRange(p1.x, p2.x)), y_(Range::validRange(p1.y, p2.y)) {
			assert(this->contains(p1) && this->contains(p2));
		}

		/// Rechteck aus X,Y,W,H
		Rect (const int x, const int y, const int width, const int height)
		 : x_( x, x + width-1 ), y_ ( y, y + height-1 ) {
		}

		Point offset() const 
		{ 
			return Point(x().start(), y().start()); 
		}
        
		/// Accessoren
		Range &x() { return x_; }
		Range &y() { return y_; }
		Range x() const { return x_; }
		Range y() const { return y_; }


		/// sind ueberhaupt sinnvolle Werte enthalten
		bool isValid() const 	{	return x().isValid() && y().isValid(); }

		/// ist die Flaeche des Rechteckes > 0
		bool isEmpty() const { return x().isEmpty() || y().isEmpty(); }

		/// Commodity-Function fuer Rechteck-Breite
		int width() { return x_.length(); }

		/// Commodity-Function fuer Rechteck-Laenge
		int height() { return y_.length(); }
		/// Prueft ob ein Punkt im Rechteck liegt
		/** \return true wenn p innerhalb des Rechteck liegt
		 */
		bool contains( Point p )  const { return ( x().contains(p.x) && y().contains(p.y)); }
		//bool containsNoBorder( Point p )  const;

		Rect scale(double p_oScaleFaktor) const;

		/** Rect-breite
		 * \return Rect-breite
		 */
		int width() const { return x().length(); }

		/** Rect-hoehe
		 * \return Rect-hoehe
		 */
		int height() const { return y().length(); }

        /**
        * Rect-Size (width, height) - excluding the last pixel (mantained for compatibility reasons)
        *  To get the size as specified in the constructor, use sizeWithBorder
        */
        Size size() const { return Size(width(),height()); }

        int widthWithBorder() const
        {
            return x_.isValid() ? x_.end() - x_.start() + 1 : 0;
        }
        
        int heightWithBorder() const
        {
            return y_.isValid() ? y_.end() - y_.start() + 1: 0;
        }
        /**
        * Rect-Size (width, height) as specified in the constructor, use sizeWithBorder
        */
        Size sizeWithBorder() const 
        {
            return Size{widthWithBorder(), heightWithBorder()};
        }


	private:
		Range x_;	///< Intervall in x-Richtung
		Range y_;	///< Intervall in y-Richtung
	};

	// -----------------------------------------------------------------------------
	// ------------------------------ Funktionen -----------------------------------
	// -----------------------------------------------------------------------------

	/// Skalierung mit ganzzahligem Faktor
	inline Rect Rect::scale(double p_oScaleFaktor) const {
		return Rect(x().scale(p_oScaleFaktor).makeValid(),
								y().scale(p_oScaleFaktor).makeValid());
	}

	/// Vergleich
	inline bool operator==( const Rect& r1, const Rect& r2 ) {
		return	( r1.x()==r2.x()) &&	(r1.y()==r2.y());
	}

	/// Vergleich
	inline bool operator!=( const Rect& r1, const Rect& r2 ) { return !(r1==r2); }

	/// Eingabe
	inline std::istream& operator>>( std::istream& os, Rect& r );

	/// Ausgabe
	inline std::ostream& operator<<( std::ostream& os, const Rect& r );

	/// Schnittmenge
	/** Liefert die Schnittmenge aus beiden Rechtecken.
	 *  Existiert kein Schnitt so wird ein Nullrechteck zurueckgegeben.
	 *  \return Schnittmenge aus Rechteck r1 und r2
	 */
	inline Rect intersect( const Rect &r1, const Rect &r2 ) {
		return Rect( r1.x() & r2.x(),  r1.y() & r2.y() );
	}

	/// Vereinigungsmenge
	/** Liefert die Vereinigungmenge (Bounding Box) beider Rechtecke.
	 *  \return Vereingungsmenge der Rechtecke r1 und r2
	 */
	inline Rect unite( const Rect &r1, const Rect &r2 ) {
		return Rect( r1.x() | r2.x(), r1.y() | r2.y() );
	}

	/**
	 * Size kann man zu Rect sinnvoll dazuaddieren ober davon abziehen
	 * um groessere/kleinere Rechtecke zu erhalten
	 */
	inline Rect operator + (Rect const& r, Size const& s)  { return Rect( r.x().dilate(s.width), r.y().dilate(s.height)); }
	inline Rect operator + (Size const& s, Rect const& r)  { return Rect( r.x().dilate(s.width), r.y().dilate(s.height)); }

	inline Rect operator - (Rect const& r, const Size& s)  { return Rect( r.x().erode(s.width), r.y().erode(s.height)); }


	inline std::ostream& operator<<( std::ostream& os, const Rect& r )	{
		os << "(" << r.x() << "x" << r.y() << ")";
		return os;
	}

	inline std::istream& operator>>( std::istream& is, Rect& r )	{
		is.ignore(5, '(');
		is >> r.x();
		is.ignore(5, 'x');
		is >> r.y();
		is.ignore(5, ')');
		return is;
	}

	inline Point offset(Rect r) { return r.offset(); }

	/**
	 * Liefert einen verkleinerten (rand <0) oder erweiterten (rand >0) Rechteck
	 * Wenn ueberhautp als Funktionausserhalb der Klasse
	 * Besser:
	 * Rand

	 */
	/** \return		neuer Rechteck
	 *	\param	xRand	Rand in x-Richtung
	 *	\param	yRand	Rand in y-Richtung
	 */
	//Rect resize(Rect const& r, const int rand) const;
	//Rect resize(Rect const& r, const int xRand,const int yRand) const;
} // namespace geo2d

template <> struct INTERFACES_API No<geo2d::Rect> { static const geo2d::Rect Value; };
} // namespace precitec

#endif //_RECT_H_
