/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2013
 *  @brief			'Seam' is the highest entity in a seam series, defined by a number. A seam holds one or more seam intervals.
 */

#ifndef SEAM_H_INCLUDED_20130724
#define SEAM_H_INCLUDED_20130724

#include <vector>

#include "common/measureTask.h"
#include "fliplib/FilterGraph.h" 
#include "message/db.interface.h"

#include "analyzer/seamInterval.h"

namespace precitec {
namespace analyzer {

/**
  *	@brief		'Seam' is the highest entity in a seam series, defined by a number. A seam holds one or more seam intervals.
  *	@ingroup Analyzer
  */
class Seam {
public:
	/**
	  *	@brief		CTOR initialized with seam number and hw parameter set id.
	  *	@param		p_oNumber			Number of seam.
	  *	@param		p_rMeasureTask		Seam interval specific measure task data (level 2 task).
	  *	@param		p_oCItFilterGraph	Iterator to graph instance in graph cache.
	  */
	   Seam(int p_oNumber, const interface::MeasureTask& p_rMeasureTask, fliplib::graph_map_t::const_iterator p_oCItFilterGraph);

	/**
	  *	@brief		Adds a new seam interval initialized with given measure task and a graph.
	  *	@param		p_rMeasureTask		Seam interval specific measure task data (level 2 task).
	  *	@param		p_oCItFilterGraph	Iterator to graph instance in graph cache.
	  */
	SeamInterval* addSeamInterval(const interface::MeasureTask& p_rMeasureTask, fliplib::graph_map_t::const_iterator p_oCItFilterGraph);
	
	/**
	  *	@brief		Returns a pointer to the graph instance in the graph map iterator.
	  */
	fliplib::FilterGraph* getGraph() const;

	const int									m_oNumber;					///< Number of seam.
	const interface::MeasureTask*				m_pMeasureTask;				///< Seam interval specific measure task data (level 2 task).
	int											m_oTriggerDelta;			///< [um] space delta between two successive triggers 
	int											m_oVelocity;				///< [um/s] Welding/ cutting velocity (user level [mm/s])
	int											m_oLength;   				///< [um] Length of seam (user level [mm]), negative = infinite
	std::vector<SeamInterval>					m_oSeamIntervals;			///< One or more seam intervals, starting by zero and ascending.
    int                                         m_oDirection;               ///< approach dircetion from upper/lower
    int                                         m_oThicknessLeft;           ///< [um] Thinckness left of blank
    int                                         m_oThicknessRight;          ///< [um] Thinckness right of blank
    int                                         m_oTargetDifference;
    int                                         m_oRoiX;
    int                                         m_oRoiY;
    int                                         m_oRoiW;
    int                                         m_oRoiH;

private:
	fliplib::graph_map_t::const_iterator		m_oCItFilterGraph;		///< Iterator to graph instance in graph cache.
};



/**
  *	@brief		Comparison operator for map insertion. Compares the seam number.
  *	@param		p_rFirst		LHS instance.
  *	@param		p_rSecond		RHS instance.
  */
bool operator<(const Seam& p_rFirst, const Seam& p_rSecond);

} // namespace analyzer
} // namespace precitec

#endif /* SEAM_H_ */
