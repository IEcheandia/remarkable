/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		Data structure which represents surface data calculated by SurfaceCalculator
*/

#ifndef STARTENDINFO_H_
#define STARTENDINFO_H_


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

		class INTERFACES_API StartEndInfo
		{

		public:
            //used in StartEndDetectionInImage
            enum class ImageState
            {
                OnlyBackground, 
                OnlyMaterial, 
                OnlyLeftEdgeVisible,
                OnlyRightEdgeVisible,
                FullEdgeVisible,
                Invalid, 
                Unknown
            };
            //used by StartEndDetectionInSeam and GeoStartEndInfo
            enum class ImageStateEvaluation
            {
                Unknown,
                BackgroundBeforeStart,
                StartEdge,
                PartialStartEdgeIgnored,
                OnlyMaterial, // includes the case of a partial edge found after a full edge
                EndEdge,
                PartialEndEdgeIgnored,
                BackgroundAfterEnd // includes the case of a partial edge found after a full edge
            };
            struct FittedLine
            {
                double m; double q;
                double getY(double x) const
                {
                    return m*x + q;
                }
                static FittedLine applyTranslation(FittedLine line, double dx, double dy)
                {
                    // the new reference system is x' = (x -dx) y' = (y -dx), the line equation becomes  y'+dy = m (x'+dx) +q -> y' = m x' + q + mdx - dy
                    line.q += (line.m*dx - dy);
                    return line;
                }
            };
            StartEndInfo();

			INTERFACES_API friend std::ostream& operator << (std::ostream& os, StartEndInfo const & v);

            ImageState m_oImageState;
            ImageStateEvaluation m_oImageStateEvaluation; //evaluated knowing the seam state

			bool isTopDark; ///< true: top is on left and/or on right side background
			bool isBottomDark; ///< true: bottom is on left and/or on right side background
            // needed by startEndRoiChecker to verify, if isTopDark=isBottomDark=true and image not full background, where the blank is
            bool isTopMaterial; ///< true: top is on left and/or on right side blank
            bool isBottomMaterial; ///< true: bottom is on left and/or on right side blank

			int threshMaterial;
			int threshBackground;

            //field needed by StartEnd ROI filters
			bool isCropped;
			int m_oStartValidRangeY;
			int m_oEndValidRangeY;

            //field needed by EdgeSkew
			int borderBgStripeY; 

            //fields needed by NotchSize
            FittedLine leftEdge;  //valid only for certain values of m_oImageStateEvaluation
            FittedLine rightEdge; //valid only for certain values of m_oImageStateEvaluation

            int imageWidth;
		};



	} // namespace geo2d
} // namespace precitec


#endif /* STARTENDINFO_H_ */
