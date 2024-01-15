
/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			WOR, HS
 *  @date			2010
 *  @brief			Traits No, Min, Max
 */


//#include <string> // wg Konstanten SHRT_MIN ...

#include "system/typeTraits.h"
#include "system/types.h"

/** \file typeTraits.cpp
 * hier werden (typisierte) Konstanten explizit definiert, damit
 * ihre Adresse genommen werden kann
 */

namespace precitec
{

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Spezialisierungen von Traits fuer diverse Typen

// No<T> macht fuer char und Bytes gar keinen Sinn,
//    fuer Shorts sehe ich im Moment keinen Verwendungszweck

//template <> const char  No<char>::Value 	 = std::numeric_limits<char>::max();
//template <> const byte   No<byte>::Value 	 = SHRstd::numeric_limits<byte>::max();
template <> const short  No<short>::Value  = std::numeric_limits<short>::max();
template <> const uShort No<uShort>::Value = std::numeric_limits<uShort>::max();
template <> const int		No<int>::Value		= std::numeric_limits<int>::max();
template <> const uInt		No<uInt>::Value		= std::numeric_limits<uInt>::max();
template <> const long		No<long>::Value		= std::numeric_limits<long>::max();
template <> const uLong		No<uLong>::Value	= std::numeric_limits<uLong>::max();
template <> const real		No<real>::Value		= std::numeric_limits<real>::quiet_NaN();
template <> const PvString	No<PvString>::Value	= PvString("No Value");
//const URange	No<URange>::Value	= URange(No<uInt>::Value, No<uInt>::Value);
//const IPoint	No<IPoint>::Value	= IPoint(No<int>::Value,  No<int>::Value);
//const IPoint  No<IPoint>::Value  = IPoint(17, 17);
template <> const float	 No<float>::Value 	= std::numeric_limits<float>::quiet_NaN();

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

template <> const char   Min<char>::Value   = std::numeric_limits<char>::min();
template <> const byte   Min<byte>::Value   = std::numeric_limits<byte>::min();
template <> const short  Min<short>::Value  = std::numeric_limits<short>::min();
template <> const uShort Min<uShort>::Value = std::numeric_limits<uShort>::min();
template <> const int    Min<int>::Value    = std::numeric_limits<int>::min();
template <> const uInt   Min<uInt>::Value   = std::numeric_limits<uInt>::min();
template <> const long   Min<long>::Value   = std::numeric_limits<long>::min();
template <> const uLong  Min<uLong>::Value  = std::numeric_limits<uLong>::min();
template <> const real 	 Min<real>::Value 	= std::numeric_limits<real>::quiet_NaN();


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

template <> const char   Max<char>::Value   = std::numeric_limits<char>::max();
template <> const byte   Max<byte>::Value   = std::numeric_limits<byte>::max();
template <> const short  Max<short>::Value  = std::numeric_limits<short>::max();
template <> const uShort Max<uShort>::Value = std::numeric_limits<uShort>::max();
template <> const int    Max<int>::Value    = std::numeric_limits<int>::max()-1;
template <> const uInt   Max<uInt>::Value   = std::numeric_limits<uInt>::max()-1;
template <> const long   Max<long>::Value   = std::numeric_limits<long>::max()-1;
template <> const uLong  Max<uLong>::Value  = std::numeric_limits<uLong>::max()-1;
template <> const real 	 Max<real>::Value 	= std::numeric_limits<real>::max();

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

template <> const char   Eps<char>::Value   = 1;
template <> const byte   Eps<byte>::Value   = 1;
template <> const short  Eps<short>::Value  = 1;
template <> const uShort Eps<uShort>::Value = 1;
template <> const int    Eps<int>::Value    = 1;
template <> const uInt   Eps<uInt>::Value   = 1;
template <> const long   Eps<long>::Value   = 1;
template <> const uLong  Eps<uLong>::Value  = 1;
template <> const float  Eps<float>::Value 	= std::numeric_limits<float>::epsilon();
template <> const real 	 Eps<real>::Value 	= std::numeric_limits<real>::epsilon();




} // namespace precitec
