/**
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			JS
*  @date			10/2014
*  @file			
*  @brief			Performs edge detection operations
*/

#ifndef EDGEDETECTIONIMPL_INCLUDED_H
#define EDGEDETECTIONIMPL_INCLUDED_H

// std lib
#include <string>

// system includes
#include <new> 
#include <cmath>

// local includes
#include "image/image.h"				///< BImage
#include "system/types.h"				// byte
#include "common/frame.h"				// ImageFrame
#include "filter/parameterEnums.h"


namespace precitec {
	namespace filter {

int roberts     (const image::BImage& p_rSource, image::BImage& p_rDestin, int pMode);
int sobel		(const image::BImage& p_rSource, image::BImage& p_rDestin, int pMode);
int kirsch		(const image::BImage& p_rSource, image::BImage& p_rDestin, int pMode);
void cannyEdgeCv(const image::BImage& imageIn, image::BImage& imageOut, int mode);



	} // namespace filter
} // namespace precitec


#endif /*EDGEDETECTIONIMPL_INCLUDED_H*/



