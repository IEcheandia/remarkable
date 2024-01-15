
/**
 *	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011-2012
 *  @brief			algorithmic interface for class TPoint
 */


#include "filter/algoPoint.h"	

#include "common/defines.h"			///< poco debug macros

#include "geo/point.h"				///< Point
#include "filter/parameterEnums.h"	///< enum ValueRankType
#include "filter/algoArray.h"		///< rank conversion

#include <tuple>					///< tuple
#include <algorithm>				///< min


namespace precitec {
	using interface::GeoPoint;	
	using interface::GeoPointarray;	
	using interface::GeoVecIntarray;
	using geo2d::Point;
	using geo2d::VecPoint;
	using geo2d::VecIntarray;
namespace filter
{


double transformX ( double inputValue, int inputTrafoX, int outTrafoX )
{
    return inputValue + inputTrafoX - outTrafoX ;
}

double transformX ( double inputValue, double inputSamplingX, int inputTrafoX, double outSamplingX, int outTrafoX )
{
    return ( inputValue / inputSamplingX  + inputTrafoX - outTrafoX ) *outSamplingX;
}

double transformY ( double inputValue, int inputTrafoY, int outTrafoY )
{
    return inputValue + inputTrafoY - outTrafoY;
}

double transformY ( double inputValue, double inputSamplingY, int inputTrafoY, double outSamplingY, int outTrafoY )
{
    return ( inputValue / inputSamplingY  + inputTrafoY - outTrafoY ) *outSamplingY;
}

geo2d::DPoint transformPoint ( geo2d::DPoint inputPoint, const interface::ImageContext& rInputContext, const interface::ImageContext& rOutputContext )
{
    geo2d::Point offsetIn {rInputContext.getTrafoX(), rInputContext.getTrafoY()};
    geo2d::Point offsetOut {rOutputContext.getTrafoX(), rOutputContext.getTrafoY()};

    return {transformX ( inputPoint.x, rInputContext.SamplingX_, offsetIn.x, rOutputContext.SamplingX_, offsetOut.x ),
            transformY ( inputPoint.y, rInputContext.SamplingY_, offsetIn.y, rOutputContext.SamplingY_, offsetOut.y ) };
}

std::pair< precitec::geo2d::DPoint, precitec::geo2d::DPoint > pointListBoundingBoxCorners(const std::vector< precitec::geo2d::DPoint >& pointList)
{
    if (pointList.empty())
    {
        return {{0.0, 0.0}, {0.0,0.0}};
    }
    geo2d::DPoint minCoordinates ( pointList.front().x, pointList.front().y) ;
    geo2d::DPoint maxCoordinates = minCoordinates;

    std::for_each(pointList.begin() + 1, pointList.end(), [&maxCoordinates, &minCoordinates](geo2d::DPoint point)
    {
        minCoordinates.x = point.x < minCoordinates.x ? point.x : minCoordinates.x;
        minCoordinates.y = point.y < minCoordinates.y ? point.y : minCoordinates.y;
        maxCoordinates.x = point.x > maxCoordinates.x ? point.x : maxCoordinates.x;
        maxCoordinates.y = point.y > maxCoordinates.y ? point.y : maxCoordinates.y;
    });
    return {minCoordinates, maxCoordinates};
}








} // namespace filter
} // namespace precitec

