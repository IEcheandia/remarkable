/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2013
 *  @brief			'SeamInterval' is an entity in a seam, defined by a number. It holds a data processing graph.
*/

#include "analyzer/seamInterval.h"
#include "module/moduleLogger.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
namespace analyzer {

SeamInterval::SeamInterval(const MeasureTask& p_rMeasureTask, int p_oNumber, int p_oStartPositionInSeam, graph_map_t::const_iterator p_oCItFilterGraph)
	: 
	m_pMeasureTask				( &p_rMeasureTask ),
	m_oNumber					( p_oNumber ),
	m_oStartPositionInSeam		( p_oStartPositionInSeam ),
	m_oEndPositionInSeam		( m_oStartPositionInSeam + m_pMeasureTask->length() ),
	m_oCItFilterGraph			( p_oCItFilterGraph )
{
	poco_assert(p_rMeasureTask.triggerDelta() != 0); // triggerDelta will be used as a divisor
	poco_assert(p_rMeasureTask.velocity() > 0);
	poco_assert(p_rMeasureTask.length() >= 0);		// length 0 is used for special crosswise action task, which is executed before seam 0
	poco_assert(p_oNumber >= -1);					// -1 is used for special crosswise action task, which is executed before seam 0
	poco_assert(p_oStartPositionInSeam >= 0);		// start pos 0 is used for special crosswise action task, which is executed before seam 0
	poco_assert(m_oEndPositionInSeam >= 0);			// end pos 0 is used for special crosswise action task, which is executed before seam 0
	poco_assert(m_oCItFilterGraph->second.get() != nullptr);
} // SeamInterval(...)



FilterGraph* SeamInterval::getGraph() const {
	const auto		pGraph	= m_oCItFilterGraph->second.get();
	poco_assert(pGraph != nullptr);
	return pGraph;
} // getGraph


} // namespace analyzer
} // namespace precitec

