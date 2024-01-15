/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		Data structure which represents data calculated by start/end detection
*/

#include "geo/startEndInfo.h"
#include <cstdio>

namespace precitec
{
	namespace geo2d
	{
		StartEndInfo::StartEndInfo()
			:
            m_oImageState(ImageState::Unknown)
		{}


		std::ostream& operator << (std::ostream& os, StartEndInfo const & v)
		{
            os << " BildStatus=" << int(v.m_oImageState);
			//os << " Length=" << v.m_oLength;
			//os << " Gradient=" << v.m_oGradient;
			//os << " GreyvalGap=" << v.m_oGreyvalGap;
			//os << " GreyvalInner=" << v.m_oGreyvalInner;
			//os << " GreyvalOuter=" << v.m_oGreyvalOuter;
			//os << " StandardDeviation=" << v.m_oStandardDeviation;
			//os << " DevLengthLeft=" << v.m_oDevelopedLengthLeft;
			//os << " DevLengthRight=" << v.m_oDevelopedLengthRight;
			return os;
		} // operator <<

	} // namespace geo2d
} // namespace precitec
