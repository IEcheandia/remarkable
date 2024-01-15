/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2013
 *  @brief			'Seam' is the highest entity in a seam series, defined by a number. A seam holds one or more seam intervals.	
 */

#include "analyzer/seam.h"
#include "module/moduleLogger.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
namespace analyzer {

Seam::Seam(int p_oNumber, const MeasureTask& p_rMeasureTask, graph_map_t::const_iterator p_oCItFilterGraph) 
	:
	m_oNumber				( p_oNumber ),
	m_pMeasureTask			( &p_rMeasureTask ),
	m_oTriggerDelta			( 0 ),
	m_oVelocity				( 0 ),	
	m_oLength				( 0 ),
	m_oDirection            ( 1 ),              /// approach dircetion from upper/lower
    m_oThicknessLeft        ( 11 ),              /// [um] Thinckness left of blank
    m_oThicknessRight       ( 22 ),              /// [um] Thinckness right of blank
    m_oTargetDifference     ( 33 ),
    m_oRoiX                 ( 0 ),
    m_oRoiY                 ( 0 ),
    m_oRoiW                 ( 0 ),
    m_oRoiH                 ( 0 ),
	m_oCItFilterGraph		( p_oCItFilterGraph )
{
    m_oDirection        = p_rMeasureTask.movingDirection();
    m_oThicknessLeft    = p_rMeasureTask.thicknessLeft();
    m_oThicknessRight   = p_rMeasureTask.thicknessRight();
    m_oTargetDifference = p_rMeasureTask.targetDifference();
    m_oRoiX             = p_rMeasureTask.roiX();
    m_oRoiY             = p_rMeasureTask.roiY();
    m_oRoiW             = p_rMeasureTask.roiW();
    m_oRoiH             = p_rMeasureTask.roiH();
    //std::cout << "\tmove Direction\t" << m_oDirection << "\n";	// DEBUG
    //std::cout << "\tThicknessLeft\t" << m_oThicknessLeft << "\n";	// DEBUG
    //std::cout << "\tThicknessRight\t" << m_oThicknessRight << "\n";	// DEBUG
}



SeamInterval* Seam::addSeamInterval(const MeasureTask& p_rMeasureTask, graph_map_t::const_iterator p_oCItFilterGraph) {
	poco_assert(p_rMeasureTask.triggerDelta() != 0); // triggerDelta will be used as a divisor

	m_oTriggerDelta		= p_rMeasureTask.triggerDelta();	// equal for each measure task within a seam
	m_oVelocity			= p_rMeasureTask.velocity();		// equal for each measure task within a seam
 
	const auto	oSeamIntervalNumber	= p_rMeasureTask.seaminterval();
	const auto	oSeamInterval		= SeamInterval( p_rMeasureTask, oSeamIntervalNumber, m_oLength, p_oCItFilterGraph ); // use length here, before next task lenght is added

	m_oLength			+= p_rMeasureTask.length();
	//std::cout << "\tseam length is now\t" << m_oLength << "\n";	// DEBUG
   
	m_oSeamIntervals.push_back(oSeamInterval); // use emplace_back as soon as both compilers support it. by now we need the assignement operator.
	poco_assert(int( m_oSeamIntervals.size() ) == oSeamInterval.m_oNumber + 1);
	poco_assert(m_oSeamIntervals[oSeamInterval.m_oNumber].m_oNumber == oSeamInterval.m_oNumber);


    
	return &m_oSeamIntervals.back();
} // addMeasureTask



FilterGraph* Seam::getGraph() const {
	const auto		pGraph	= m_oCItFilterGraph->second.get();
	poco_assert(pGraph != nullptr);
	return pGraph;
} // getGraph



bool operator<(const Seam& p_rFirst, const Seam& p_rSecond) {
	return p_rFirst.m_oNumber < p_rSecond.m_oNumber;
} // operator<


} // namespace analyzer
} // namespace precitec

