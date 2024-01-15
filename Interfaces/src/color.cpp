/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, HS
 * 	@date		2010
 *	@brief		Defines overlay color.
 */


#include "overlay/color.h"

namespace precitec
{
	namespace image
	{

		const Color Color::Red() { return Color(0xFF0000); }
		const Color Color::Green(){ return Color(0x00FF00); }
		const Color Color::Blue(){ return Color(0x0000FF); }
		const Color Color::White(){ return Color(0xFFFFFF); }
		const Color Color::Black(){ return Color(0x000000); }
		const Color Color::Yellow(){ return Color(0xFFFF00); }
		const Color Color::Cyan(){ return Color(0x00FFFF); }
		const Color Color::Orange(){ return Color(0xFFA500); }
		const Color Color::Magenta(){ return Color(0xFF00FF); }

		/*static*/ const Color Color::m_oButter				= Color(0xfce94f);	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		/*static*/ const Color Color::m_oOrange				= Color(0xfcaf3e);	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		/*static*/ const Color Color::m_oOrangeDark			= Color(0xf57900);	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		/*static*/ const Color Color::m_oChocolate			= Color(0xe9b96e);	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		/*static*/ const Color Color::m_oChameleon			= Color(0x8ae234);	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		/*static*/ const Color Color::m_oChameleonDark		= Color(0x73d216);	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		/*static*/ const Color Color::m_oChameleonDarker	= Color(0x4e9a06);	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		/*static*/ const Color Color::m_oSkyBlue			= Color(0x729fcf);	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		/*static*/ const Color Color::m_oSkyBlueDark		= Color(0x3465a4);	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		/*static*/ const Color Color::m_oPlum				= Color(0xad7fa8);	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		/*static*/ const Color Color::m_oScarletRed			= Color(0xef2929);	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		/*static*/ const Color Color::m_oAluminium0			= Color(0xeeeeec);	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		/*static*/ const Color Color::m_oAluminium1			= Color(0xd3d7cf);	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		/*static*/ const Color Color::m_oAluminium2			= Color(0xbabdb6);	// http://de.wikipedia.org/wiki/Tango_Desktop_Project
		/*static*/ const Color Color::m_oMagenta			= Color(0xff00ff);

	} // namespace image
} // namespace precitec
