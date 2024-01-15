
/**
 *	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011-2012
 *  @brief			algorithmic interface for class TPoint
 */



#ifndef ALGOPOINT_H_20111020
#define ALGOPOINT_H_20111020


#include "Analyzer_Interface.h"

#include "geo/array.h"				///< array data structure

#include "geo/geo.h"				///< GeoPoint

#include <functional>				///< function
#include <tuple>					///< tuple


namespace precitec {
namespace filter {

std::pair<geo2d::DPoint, geo2d::DPoint> pointListBoundingBoxCorners(const std::vector<geo2d::DPoint> & pointList);



double transformX (double inputValue, int inputTrafoX, int outTrafoX );
double transformX (double inputValue, double inputSamplingX, int inputTrafoX, double outSamplingX, int outTrafoX );
double transformY (double inputValue, int inputTrafoY, int outTrafoY );
double transformY (double inputValue, double inputSamplingY, int inputTrafoY, double outSamplingY, int outTrafoY );
geo2d::DPoint transformPoint (geo2d::DPoint inputPoint, const interface::ImageContext& rInputContext, const interface::ImageContext& rOutputContext );



} // namespace filter
} // namespace precitec


#endif // ALGOPOINT_H_20111020
