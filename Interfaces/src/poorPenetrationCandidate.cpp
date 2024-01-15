/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		Data structure which represents a poor penetration candidate.
*/

#include "geo/poorPenetrationCandidate.h"
#include <cstdio>

namespace precitec
{
	namespace geo2d
	{
		PoorPenetrationCandidate::PoorPenetrationCandidate()
			:
			m_oLength(0),
			m_oWidth(0),
			m_oGradient(0),
			m_oGreyvalGap(0),
			m_oGreyvalInner(0),
			m_oGreyvalOuter(0),
			m_oStandardDeviation(0),
			m_oDevelopedLengthLeft(0),
			m_oDevelopedLengthRight(0)
		{}

		PoorPenetrationCandidate::PoorPenetrationCandidate(int width, int length, int gradient, int greyvalGap, int greyvalInner, int greyvalOuter,
			int standardDeviation, int developedLenghtLeft, int developedLenghtRight):
			
			m_oLength(length),
			m_oWidth(width),
			m_oGradient(gradient),
			m_oGreyvalGap(greyvalGap),
			m_oGreyvalInner(greyvalInner),
			m_oGreyvalOuter(greyvalOuter),
			m_oStandardDeviation(standardDeviation),
			m_oDevelopedLengthLeft(developedLenghtLeft),
			m_oDevelopedLengthRight(developedLenghtRight)
		{}

		inline bool PoorPenetrationCandidate::operator == (const PoorPenetrationCandidate& p_rOther) const 
		{

			
			if (p_rOther.m_oWidth != m_oWidth)		return false;
			if (p_rOther.m_oLength != m_oLength)	return false;
			if (p_rOther.m_oGradient != m_oGradient)		return false;
			if (p_rOther.m_oGreyvalGap != m_oGreyvalGap)		return false;
			if (p_rOther.m_oGreyvalInner != m_oGreyvalInner)	return false;
			if (p_rOther.m_oGreyvalOuter != m_oGreyvalOuter)		return false;
			if (p_rOther.m_oStandardDeviation != m_oStandardDeviation)		return false;
			if (p_rOther.m_oDevelopedLengthLeft != m_oDevelopedLengthLeft)	return false;
			if (p_rOther.m_oDevelopedLengthRight != m_oDevelopedLengthRight)		return false;
			return true;
		} // operator ==

		std::ostream& operator << (std::ostream& os, PoorPenetrationCandidate const & v) 
		{

			
			os << " Width=" << v.m_oWidth;
			os << " Length=" << v.m_oLength;
			os << " Gradient=" << v.m_oGradient;
			os << " GreyvalGap=" << v.m_oGreyvalGap;
			os << " GreyvalInner=" << v.m_oGreyvalInner;
			os << " GreyvalOuter=" << v.m_oGreyvalOuter;
			os << " StandardDeviation=" << v.m_oStandardDeviation;
			os << " DevLengthLeft=" << v.m_oDevelopedLengthLeft;
			os << " DevLengthRight=" << v.m_oDevelopedLengthRight;
			return os;
		} // operator <<

	} // namespace geo2d
} // namespace precitec
