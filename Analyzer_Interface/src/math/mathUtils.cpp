/*
 * MathUtils.cpp
 *
 *  Created on: Sep 17, 2013
 *      Author: abeschorner
 */

#include "math/mathUtils.h"
#include "math/mathCommon.h"
#include <cmath>

namespace precitec {
namespace math {

bool lengthRatio(double &p_rRes, const double p_oSlope, const double p_oLength, const double p_oLengthToCompareTo, tVecDouble &p_rX)
{
	double oLength=p_oLengthToCompareTo;
	if (std::abs(p_oLength) < math::eps)
	{
		double oAngle = std::atan(p_oSlope);

		if (std::abs(oAngle) >= math::eps)
		{
			oLength = oLength / std::abs(std::cos(oAngle));
		}
	}
	if (oLength < math::eps)
	{
		return false;
	}

	p_rRes = ((p_rX[1] - p_rX[0])/oLength);
	return true;
}

bool isPowerOfTwo(unsigned long long x)
{
	return (x & (x - 1)) == 0;
}


} /* namespace math */
} /* namespace precitec */
