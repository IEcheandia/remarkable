#ifndef _SIZE_H_
#define _SIZE_H_
/** \file 	size.h
 *  \author Sascha Demirovic
 *  \date
 */

#include <iostream>

namespace precitec
{
namespace geo2d
{

	/// Abmessungsklasse
	/**
	 */
	class Size {
	public:
		/// Nullsize
		Size() : width(0), height(0) {}

		/// Size
		Size( int w, int h ) : width(w), height(h) {}

		/// Flaeche
		int area() const { return width * height; }

		/// vergroesserte Size ausgeben
		Size operator + ( Size const& s) const { return Size( width+s.width, height+s.height); }

		/// verkleinerte Size ausgeben
		Size operator - ( Size const& s) const { return Size( width-s.width, height-s.height); }

		/// Ausgabe
		friend 			// friend oder aus der Klasse + inline
		std::ostream& operator << ( std::ostream& os, Size const& s )	{
			os << "[" << s.width << "|" << s.height << "]";
			return os;
		}

		/// Eingabe sollte konform zu Ausgabe sein
		friend			// friend oder aus der Klasse + inline
		std::istream& operator>> ( std::istream& is, Size& s )	{
			char ch;
			is >> ch >> s.width >> ch >> s.height >> ch;
			return is;
		}

	public:
		int width;		///< Breite
		int height;		///< Hoehe
	};


	// -----------------------------------------------------------------------------
	// ------------------------------ Funktionen -----------------------------------
	// -----------------------------------------------------------------------------
	/// Vergleiche nur minimal wg std::Container
	inline bool operator == ( const Size& s1, const Size& s2 ) { return ( s1.width == s2.width && s1.height == s2.height ); }
	inline bool operator != ( const Size& s1, const Size& s2 ) { return ! (s1 == s2) ;}

	inline bool operator  < ( const Size& s1, const Size& s2 ) { return s1.area() <  s2.area(); }
	//inline bool operator  > ( const Size& s1, const Size& s2 ) { return s1.area() >  s2.area(); }
	//inline bool operator <= ( const Size& s1, const Size& s2 ) { return s1.area() <= s2.area(); }
	//inline bool operator >= ( const Size& s1, const Size& s2 ) { return s1.area() >= s2.area(); }


} // namespace geo2d
} // namespace precitec

#endif //_SIZE_H_
