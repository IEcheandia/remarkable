/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2016
 * 	@brief		This filter extracts the rank from a double value.
 */

#ifndef EXTRACTRANK_H_
#define EXTRACTRANK_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

namespace precitec {
namespace filter {

/**
 * @brief This filter extracts the rank from a double value.
 */
class FILTER_API ExtractRank : public fliplib::TransformFilter
{
public:

	/**
	 * CTor.
	 */
	ExtractRank();
	/**
	 * @brief DTor.
	 */
	virtual ~ExtractRank();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeOutName;		///< Pipe: Data out-pipe.

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

protected:

	/**
	 * @brief In-pipe registration.
	 * @param p_rPipe Reference to pipe that is getting connected to the filter.
	 * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
	 */
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/**
	 * @brief Processing routine.
	 * @param p_pSender pointer to
	 * @param p_rEvent
	 */
	void proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rE );

protected:

	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInData;			///< Data in-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOutData;			///< Data out-pipe.

}; // class ExtractRank

} // namespace filter
} // namespace precitec

#endif /* EXTRACTRANK_H_ */
