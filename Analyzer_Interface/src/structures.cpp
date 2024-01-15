/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		AB
 * 	@date		2011
 * 	@brief		SigChange structure
 */


#include <filter/structures.h>

namespace precitec {
	namespace filter {


	TRunOrientation valueToOrientation(const int p_oVal)
	{
		switch (p_oVal)
		{
			case (int)(eOrientationConcave):
			{
				return eOrientationConcave;
			}
			case (int)(eOrientationConvex):
			{
				return eOrientationConvex;
			}
			default:
			{
				return eOrientationInvalid;
			}
		}
	}


	TRunOrientation valueToOrientation(const double p_oVal)
	{
		int oVal = (int)p_oVal;
		return valueToOrientation(oVal);
	}

	} // namespace filter
} // namespace precitec


