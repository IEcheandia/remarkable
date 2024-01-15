/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2013
 *  @brief			'SeamSeries' is the highest entity in a product, defined by a number. A seam series holds one ore more seams.
 */

#ifndef SEAMSERIES_H_INCLUDED_20130724
#define SEAMSERIES_H_INCLUDED_20130724

#include <map>

#include "common/measureTask.h"
#include "fliplib/FilterGraph.h" 
#include "message/db.interface.h"

#include "analyzer/seam.h"

namespace precitec {
namespace analyzer {

/**
  *	@brief		'SeamSeries' is the highest entity in a product, defined by a number. A seam series holds one ore more seams.
  *	@ingroup Analyzer
  */
class SeamSeries {
public:

	/**
	  *	@brief		CTOR initialized with seam series number and hw parameter set ID.
	  *	@param		p_oNumber			Number of seam.
	  *	@param		p_rHwParamSetId		Hardware parameter set ID.
	  */
	explicit SeamSeries(int p_oNumber, const Poco::UUID	p_rHwParamSetId);

	/**
	  *	@brief		Adds a new seam initialized with given measure task.
	  *	@param		p_rMeasureTask		Seam specific measure task data (level 1 task).
	  *	@param		p_oCItFilterGraph	Iterator to graph instance in graph cache.
	  */
	Seam* addSeam(const interface::MeasureTask& p_rMeasureTask, fliplib::graph_map_t::const_iterator p_oCItFilterGraph);

// private:
	const int			m_oNumber;			///< Number of seam series.
	const Poco::UUID	m_oHwParamSetId;	///< Hardware parameter set ID.
	std::map<int, Seam>	m_oSeams;			///< One or more seams.
};


/**
  *	@brief		Comparison operator for map insertion. Compares the seam series number.
  *	@param		p_rFirst		LHS instance.
  *	@param		p_rSecond		RHS instance.
  */
bool operator<(const SeamSeries& p_rFirst, const SeamSeries& p_rSecond);

} // namespace analyzer
} // namespace precitec

#endif /* SEAMSERIES_H_ */
