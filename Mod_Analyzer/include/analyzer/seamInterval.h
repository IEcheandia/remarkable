/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2013
 *  @brief			'SeamInterval' is an entity in a seam, defined by a number. It holds a data processing graph.
 */

#ifndef SEAMINTERVAL_H_INCLUDED_20130724
#define SEAMINTERVAL_H_INCLUDED_20130724

#include "fliplib/FilterGraph.h" 

#include "common/measureTask.h" 

namespace precitec {
namespace analyzer {

/**
  *	@brief	'SeamInterval' is an entity in a seam, defined by a number. It holds a data processing graph.
  *	@ingroup Analyzer
  */
class SeamInterval {
public:

	/**
	  *	@brief		CTOR initialized with a measure task, a graph, a seam interval number and the start position in the overlying seam.
	  *	@param		p_rMeasureTask			Seam interval specific measure task data (level 2 task).
	  *	@param		p_oCItFilterGraph		Iterator to graph instance in graph cache.
	  *	@param		p_oNumber				Number of seam.
	  *	@param		p_oStartPositionInSeam	[um] Start position in the overlying seam.
	  */
	SeamInterval(const interface::MeasureTask& p_rMeasureTask, int p_oNumber, int p_oStartPositionInSeam, fliplib::graph_map_t::const_iterator p_oCItFilterGraph);

	/**
	  *	@brief		Returns a pointer to the graph instance in the graph map iterator.
	  */
	fliplib::FilterGraph* getGraph() const;

	// nb: const not possible for not deleting assignment operator, which is necessary for push_back in vector
	const interface::MeasureTask*				m_pMeasureTask;				///< Seam interval specific measure task data (level 2 task).
	int											m_oNumber;					///< Number of seam interval.
	int											m_oStartPositionInSeam;		///< [um] Start position in the overlying seam.
	int											m_oEndPositionInSeam;		///< [um] End position in the overlying seam.

private:
	fliplib::graph_map_t::const_iterator		m_oCItFilterGraph;		///< Iterator to graph instance in graph cache.
};

} // namespace analyzer
} // namespace precitec

#endif /* SEAMINTERVAL_H_ */
