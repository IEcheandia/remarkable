/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		Data structure which represents a hough poor penetration candidate
*/

#ifndef HOUGHPOORPENETRATIONCANDIDATE_H_
#define HOUGHPOORPENETRATIONCANDIDATE_H_


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

		class INTERFACES_API HoughPPCandidate
		{

		public:
			HoughPPCandidate();
			HoughPPCandidate(bool twoLinesFound, unsigned int meanBrightness, bool lineIntersection, 
				unsigned int position1, unsigned int position2, 
				unsigned int diffToMiddle1, unsigned int diffToMiddle2, 
				double numberOfPixelOnLine1, double numberOfPixelOnLine2, 
				unsigned int biggestInterruption1, unsigned int biggestInterruption2
			);

			bool operator == (const HoughPPCandidate& p_rOther) const;

			INTERFACES_API friend std::ostream& operator << (std::ostream& os, HoughPPCandidate const & v);

			unsigned int m_oWidth;		///
			unsigned int m_oLength;		///

			//generell
			bool m_oTwoLinesFound; // wurden ueberhaupt zwei Linien gefunden?
			unsigned int m_oMeanBrightness; // mittlere Helligkeit zwischen den Linien
			bool m_oLineIntersection; // gibt an, ob sich die beiden Linien im ROI schneiden

			// Geradenbezogen
			unsigned int m_oPosition1; // Position im ROI
			unsigned int m_oPosition2;

			unsigned int m_oDiffToMiddle1; // DIfferenz zur Mitte
			unsigned int m_oDiffToMiddle2;

			double m_oNumberOfPixelOnLine1; // Anzahl Pixel auf der Linie
			double m_oNumberOfPixelOnLine2;

			unsigned int m_oBiggestInterruption1; //groesste zusammenhaengende Unterbrechung
			unsigned int m_oBiggestInterruption2;

			unsigned int m_oCountInterruptions1; // Anzahl Unterbrechungen
			unsigned int m_oCountInterruptions2;

			unsigned int m_oHeightRoi;		///
		};



	} // namespace geo2d
} // namespace precitec


#endif /* HOUGHPOORPENETRATIONCANDIDATE_H_ */
