/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		Data structure which represents a hough poor penetration candidate.
*/

#include "geo/houghPPCandidate.h"
#include <cstdio>

namespace precitec
{
	namespace geo2d
	{
		HoughPPCandidate::HoughPPCandidate()
			:
			m_oWidth(0),		///
			m_oLength(0),		///

			m_oTwoLinesFound(false),
			m_oMeanBrightness(0),
			m_oLineIntersection(false),
			m_oPosition1(0),
			m_oPosition2(0),
			m_oDiffToMiddle1(0),
			m_oDiffToMiddle2(0),
			m_oNumberOfPixelOnLine1(0),
			m_oNumberOfPixelOnLine2(0),
			m_oBiggestInterruption1(0),
			m_oBiggestInterruption2(0)
		{}

		HoughPPCandidate::HoughPPCandidate(bool twoLinesFound, unsigned int meanBrightness, bool lineIntersection,
			unsigned int position1, unsigned int position2,
			unsigned int diffToMiddle1, unsigned int diffToMiddle2,
			double numberOfPixelOnLine1, double numberOfPixelOnLine2,
			unsigned int biggestInterruption1, unsigned int biggestInterruption2
			)
		{
			m_oWidth = 0;
			m_oLength = 0;

			m_oTwoLinesFound = twoLinesFound;
			m_oMeanBrightness = meanBrightness;
			m_oLineIntersection = lineIntersection;
			m_oPosition1 = position1;
			m_oPosition2 = position2;
			m_oDiffToMiddle1 = diffToMiddle1;
			m_oDiffToMiddle2 = diffToMiddle2;
			m_oNumberOfPixelOnLine1 = numberOfPixelOnLine1;
			m_oNumberOfPixelOnLine2 = numberOfPixelOnLine2;
			m_oBiggestInterruption1 = biggestInterruption1;
			m_oBiggestInterruption2 = biggestInterruption2;
		}

		inline bool HoughPPCandidate::operator == (const HoughPPCandidate& p_rOther) const
		{
			if (p_rOther.m_oTwoLinesFound != m_oTwoLinesFound)		return false;
			if (p_rOther.m_oMeanBrightness != m_oMeanBrightness)	return false;
			if (p_rOther.m_oLineIntersection != m_oLineIntersection)		return false;
			if (p_rOther.m_oPosition1 != m_oPosition1)	return false;
			if (p_rOther.m_oPosition2 != m_oPosition2)		return false;
			if (p_rOther.m_oDiffToMiddle1 != m_oDiffToMiddle1)	return false;
			if (p_rOther.m_oDiffToMiddle2 != m_oDiffToMiddle2)		return false;
			if (p_rOther.m_oNumberOfPixelOnLine1 != m_oNumberOfPixelOnLine1)	return false;
			if (p_rOther.m_oNumberOfPixelOnLine2 != m_oNumberOfPixelOnLine2)		return false;
			if (p_rOther.m_oBiggestInterruption1 != m_oBiggestInterruption1)	return false;
			if (p_rOther.m_oBiggestInterruption2 != m_oBiggestInterruption2)		return false;
			return true;
		} // operator ==

		std::ostream& operator << (std::ostream& os, HoughPPCandidate const & v)
		{
			os << " TwoLinesFound=" << v.m_oTwoLinesFound;
			os << " MeanBrightness=" << v.m_oMeanBrightness;
			os << " LineIntersection=" << v.m_oLineIntersection;
			os << " Position1=" << v.m_oPosition1;
			os << " Position2=" << v.m_oPosition2;
			os << " DiffToMiddle1=" << v.m_oDiffToMiddle1;
			os << " DiffToMiddle2=" << v.m_oDiffToMiddle2;
			os << " NumberOfPixelOnLine1=" << v.m_oNumberOfPixelOnLine1;
			os << " NumberOfPixelOnLine2=" << v.m_oNumberOfPixelOnLine2;
			os << " BiggestInterruption1=" << v.m_oBiggestInterruption1;
			os << " BiggestInterruption2=" << v.m_oBiggestInterruption2;

			return os;
		} // operator <<

	} // namespace geo2d
} // namespace precitec
