/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		Data structure which represents a poor penetration candidate
*/

#ifndef POORPENETRATIONCANDIDATE_H_
#define POORPENETRATIONCANDIDATE_H_


#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "InterfacesManifest.h"

namespace precitec
{
	namespace geo2d
	{
		                     
		class INTERFACES_API PoorPenetrationCandidate
		{

		public:
			PoorPenetrationCandidate();
			PoorPenetrationCandidate(int width, int length, int gradient, int greyvalGap, int greyvalInner, int greyvalOuter, 
				int standardDeviation, int developedLenghtLeft, int developedLenghtRight);

			bool operator == (const PoorPenetrationCandidate& p_rOther) const;

			INTERFACES_API friend std::ostream& operator << (std::ostream& os, PoorPenetrationCandidate const & v);

			unsigned int xmin,ymin,xmax,ymax;
			unsigned int m_oLength;		///
			unsigned int m_oWidth;		///
			unsigned int m_oGradient;		///
			unsigned int m_oGreyvalGap;		///
			unsigned int m_oGreyvalInner;		///
			unsigned int m_oGreyvalOuter;		///
			unsigned int m_oStandardDeviation;		///
			unsigned int m_oDevelopedLengthLeft;		///
			unsigned int m_oDevelopedLengthRight;		///

		};



	} // namespace geo2d
} // namespace precitec


#endif /* POORPENETRATIONCANDIDATE_H_ */
