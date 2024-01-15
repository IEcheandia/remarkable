
/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			WOR, HS
 *  @date			2010
 *  @brief			Traits No, Min, Max
 */


#ifndef TYPETRAITS_H_
#define TYPETRAITS_H_

#include "system/types.h"
#include "SystemManifest.h"
#include "SystemManifest.h"
#undef min
#undef max
/// @cond HIDDEN_SYMBOLS
#include <limits>
/// @endcond

// Traits No, Min, Max

namespace precitec
{
	/** 
	 * pro Typ wird ein Wert reserviert als
	 * als  InValid-Bezeichner oder Wert nicht vorhanden/gesetzt
	 * noetig u.a. fuer Datenbank
	 */
	template <class T> struct SYSTEM_API No { static const T Value; };

	/** 
	 * dieser Wert ist kleiner als alle anderen Werte des Typs
	 * gedacht fuer Suchen und Min/Max initialisierungen
	 */
	template <class T> struct SYSTEM_API Min { static const T Value; };
	
	/** 
	 * dieser Wert ist kleiner als alle anderen Werte des Typs
	 * gedacht fuer Suchen und Min/Max initialisierungen
	 */
	template <class T> struct SYSTEM_API Max { static const T Value; };
	
	/** 
	 * dieser Wert ist der Abstand zweeier aufeinanderfolgender Zahlen
	 * gedacht fuer Abfragen auf Null (<Epsilon)
	 */
	template <class T> struct SYSTEM_API Eps { static const T Value; };

	/**
	 * ist ein Typ ein POD oder etwas anderes
	 * gedacht etwa fuer swap
	 */
	template <class T> struct SYSTEM_API IsPod { enum {Value=0}; };
	template <> struct IsPod<char> { enum {Value=1}; };
	template <> struct IsPod<byte> { enum {Value=1}; };
	template <> struct IsPod<short> { enum {Value=1}; };
	template <> struct IsPod<uShort> { enum {Value=1}; };
	template <> struct IsPod<int> { enum {Value=1}; };
	template <> struct IsPod<uInt> { enum {Value=1}; };
	template <> struct IsPod<long> { enum {Value=1}; };
	template <> struct IsPod<uLong> { enum {Value=1}; };
	template <> struct IsPod<float> { enum {Value=1}; };
	template <> struct IsPod<double> { enum {Value=1}; };
	template <> struct IsPod<long double> { enum {Value=1}; };
	template <> struct IsPod<long long> { enum {Value=1}; };
	template <> struct IsPod<unsigned long long> { enum {Value=1}; };

} // namespace precitec
#endif /*TYPETRAITS_H_*/
