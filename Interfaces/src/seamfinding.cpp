/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		Data structure which represents a single seam finding.
*/



#include "geo/seamfinding.h"
#include <cstdio>


namespace precitec 
{
	namespace geo2d 
	{

		// constructor
		SeamFinding::SeamFinding()
			:
			m_oSeamLeft(0),
			m_oSeamRight(0),	
			m_oAlgoType(0),
			m_oQuality(0)
		{}

		SeamFinding::SeamFinding(int seamLeft, int seamRight, int algoType, int quality)
		{
			m_oSeamLeft = seamLeft;
			m_oSeamRight = seamRight;
			m_oAlgoType = algoType;
			m_oQuality = quality;
		}

		inline bool SeamFinding::operator == (const SeamFinding& p_rOther) const {
			if (p_rOther.m_oSeamLeft != m_oSeamLeft)		return false;
			if (p_rOther.m_oSeamRight != m_oSeamRight)	return false;
			if (p_rOther.m_oAlgoType != m_oAlgoType)		return false;
			return true;
		} // operator ==

		std::ostream& operator << (std::ostream& os, SeamFinding const & v) {
			os << " SeamLeft=" << v.m_oSeamLeft;
			os << " SeamRight=" << v.m_oSeamRight;
			os << " AlgoType=" << v.m_oAlgoType;
			return os;
		} // operator <<

	} // namespace geo2d
} // namespace precitec
