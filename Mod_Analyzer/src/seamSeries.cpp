/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2013
 *  @brief			'SeamSeries' is the highest entity in a product, defined by a number. A seam series holds one ore more seams.
 */

#include "analyzer/seamSeries.h"
#include "module/moduleLogger.h"

using namespace fliplib;
using Poco::UUID;
namespace precitec {
	using namespace interface;
namespace analyzer {

SeamSeries::SeamSeries(int p_oNumber, const UUID p_rHwParamSetId) 
	:
	m_oNumber			( p_oNumber ),
	m_oHwParamSetId		( p_rHwParamSetId )
{
}



Seam*  SeamSeries::addSeam(const MeasureTask& p_rMeasureTask, graph_map_t::const_iterator p_oCItFilterGraph) {
	const auto	oSeamNumber			= p_rMeasureTask.seam();
	const auto	oSeam				= Seam		( oSeamNumber, p_rMeasureTask, p_oCItFilterGraph );
	auto&		rSeam				= m_oSeams.insert(std::map<int, Seam>::value_type(oSeamNumber, oSeam)).first->second;
	return &rSeam;
} // addMeasureTask



bool operator<(const SeamSeries& p_rFirst, const SeamSeries& p_rSecond) {
	return p_rFirst.m_oNumber < p_rSecond.m_oNumber;
} // operator<


} // namespace analyzer
} // namespace precitec

