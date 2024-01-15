/*!
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			Simon Hilsenbeck (HS)
*  @date			2011
*  @file
*  @brief			Provides lookup table for trigonometric sin and cos functions.
*/

#ifndef LOOKUP_20110830_H_
#define LOOKUP_20110830_H_


#include <cmath>						///< trigonometry, pi constant
#include <array>						///< static array
#include "filter/algoStl.h"

namespace precitec {
	namespace filter {


///  Lookup table for trigonometric functions.
/**
* Holds two fixed size arrays that contain cos and sin values.
* Generated at construction.
* Size defined at compile time.
*/template<std::size_t N>
struct Lookup {
	Lookup() {
	static_assert(N % 2 == 0, "Size is not an even number.");
	const double	oDPhi	= 2*M_PI / N; // delta phi for angels [-PI; PI]
	double			oAngle	= -M_PI;
	poco_assert_dbg(m_oCos.size() == m_oSin.size());
	for(auto itSin = m_oSin.begin(), itCos = m_oCos.begin(); itSin != m_oSin.end(); ++itSin, ++itCos) {
			*itSin = std::sin( oAngle );
			*itCos = std::cos( oAngle );		
			oAngle += oDPhi;
		} // for
	} // Lookup()
	std::size_t size() const {return N;};
	std::array<double, N> m_oSin, m_oCos;
}; // Lookup



/// map angle [-PI; +PI] to lookup index. Precondition: lookup has even size and angle range is [-PI; +PI]
template <std::size_t N> inline int
angle2index(double p_oAngle) {
	static const int	oHalfOffset	= N / 2; // index that represents angle = 0
	// from 0 angle add or subtract normalized and scaled angle 
	return oHalfOffset + roundToT<int>( oHalfOffset * (p_oAngle / M_PI) );
}



/// map lookup index to angle [-PI; +PI]. Precondition: lookup has even size and angle range is [-PI; +PI]
template <std::size_t N> inline double
index2angle(int p_oIndex) {
	static const int	oHalfOffset	= N / 2; // index that represents angle = 0
	// shift index and scale to angle
	return (p_oIndex - oHalfOffset) * (M_PI / oHalfOffset);
}


	} // namespace filter
} // namespace precitec



#endif /*LOOKUP_20110830*/

