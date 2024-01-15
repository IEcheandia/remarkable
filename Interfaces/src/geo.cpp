/**
*	@file
*	@copyright	Precitec Vision GmbH & Co. KG
*	@author		Wor, HS
*	@date		2011
*	@brief		TGeo, typedefs, global rank double[0.0, 1.0]
*/

#include "geo/geo.h"

namespace precitec {
	using namespace geo2d;
namespace interface {

const double  NotPresent	=	0.0;
const double  Minimum		=	0.01;
const double  Marginal		=	0.1;
const double  Bad			=	0.25;
const double  Doubtful		=	0.4;
const double  NotGood		=	0.5;
const double  Ok			=	0.60;
const double  Good			=	0.75;
const double  Perfect		=	0.9;
const double  Limit			=	1.0;

std::ostream &operator <<(std::ostream &os, RegTypes const& r) {
	switch (r) {
		case RegInt:		os << "RegInt"; break;
		case RegDouble:		os << "RegDouble"; break;
		case RegRange:		os << "RegRange"; break;
		case RegRange1d:	os << "RegRange1d"; break;
		case RegPoint:		os << "RegPoint"; break;
		default:			os << "NotReg"; break;
	}
	return os;
} // operator <<



	template <> const int  RegisteredType<int>::Value 		= RegInt;	// deprecated
	template <> const int  RegisteredType<double>::Value 	= RegDoubleArray;
	template <> const int  RegisteredType<Range>::Value 	= RegRange;	// deprecated
	template <> const int  RegisteredType<Range1d>::Value 	= RegRange1d;	// deprecated
	template <> const int  RegisteredType<Point>::Value 	= RegPoint;	// deprecated
	/// default-Element = Num registrierte Elemente
	template <> const int  RegisteredType<char>::Value 		= NumRegTypes;

} // namespace inspect
} // namespace precitec
