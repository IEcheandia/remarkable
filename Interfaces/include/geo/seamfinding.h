/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		Data structure which represents a single seam finding.
*/

#ifndef SEAMFINDING_H_
#define SEAMFINDING_H_


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

		class INTERFACES_API SeamFinding 
		{

		public:
			SeamFinding();
			SeamFinding(int seamLeft, int seamRight, int algoType, int quality);

			bool operator == (const SeamFinding& p_rOther) const;

			INTERFACES_API friend std::ostream& operator << (std::ostream& os, SeamFinding const & v);

			unsigned int m_oSeamLeft;		///
			unsigned int m_oSeamRight;		///
			unsigned int m_oAlgoType;		///
			unsigned int m_oQuality;		///

		};



	} // namespace geo2d
} // namespace precitec


#endif /* SEAMFINDING_H_ */
