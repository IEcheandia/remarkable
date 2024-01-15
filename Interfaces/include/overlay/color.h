/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, HS
 * 	@date		2010
 *	@brief		Defines overlay color.
 */

#ifndef COLOR_H_20131210_INCLUDED
#define COLOR_H_20131210_INCLUDED

#include <iostream>

#include "system/types.h"

#include "InterfacesManifest.h"

namespace precitec {
namespace image {

	struct INTERFACES_API Color
	{
		Color() : red(0xFF), green(0xFF), blue(0xFF), alpha(0xFF) {}
		Color(byte r, byte g, byte b, byte a) : red(r), green(g), blue(b), alpha(a) {}
		Color(byte r, byte g, byte b) : red(r), green(g), blue(b), alpha(0xFF) {}
		Color(int rgb)	:
			red		( (rgb & 0xFF0000) / 0x10000),
			green 	( (rgb & 0xFF00) / 0x100 ),
			blue	( (rgb & 0xFF) ),
			alpha	(0xFF)
		{}

		byte red;
		byte green;
		byte blue;
		byte alpha;

		//The byte-ordering of the 32-bit ARGB value is AARRGGBB.
		// The most significant byte (MSB), represented by AA, is the alpha component value.
		// The second, third, and fourth bytes, represented by RR, GG, and BB,
		// respectively, are the color components red, green, and blue, respectively
		unsigned int toARGB() const { return ( int(alpha) * 0x1000000 +  int(red) * 0x10000 +  int(green) * 0x100 +  int(blue)); }
		friend std::ostream &operator <<(std::ostream &os, Color const& c) {
			os << "C: [" << std::hex <<int(c.toARGB()) << "]";
			return os;
		}
		static const Color Red();
		static const Color Green();
		static const Color Blue();
		static const Color White();
		static const Color Black();
		static const Color Yellow();
		static const Color Cyan();
		static const Color Orange();
		static const Color Magenta();

		static const Color	m_oButter;			// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		static const Color	m_oOrange;			// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		static const Color	m_oOrangeDark;			// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		static const Color	m_oChocolate;		// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		static const Color	m_oChameleon;		// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		static const Color	m_oChameleonDark;	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		static const Color	m_oChameleonDarker;	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		static const Color	m_oSkyBlue;			// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		static const Color	m_oSkyBlueDark;		// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		static const Color	m_oPlum;			// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		static const Color	m_oScarletRed;		// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		static const Color	m_oAluminium0;		// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		static const Color	m_oAluminium1;		// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		static const Color	m_oAluminium2;		// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		static const Color	m_oMagenta;
	};


} // namespace image
} // namespace precitec

#endif /*COLOR_H_20131210_INCLUDED*/
