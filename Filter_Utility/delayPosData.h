/**
* 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		SK
 * 	@date		2015
 * 	@brief		This filter delays the output of data elements.
 *              The delay depends on second pipe, which comes from an incremental encoder or something equal.
  */

#ifndef DELAY_POSDATA_H_
#define DELAY_POSDATA_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>
// stl includes
#include <deque>

namespace precitec {
namespace filter {

/**
 * @brief This filter delays the output of data elements.
 */
class FILTER_API DelayPosData : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	DelayPosData();
	/**
	 * @brief DTor.
	 */
	virtual ~DelayPosData();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeOutName;		///< Pipe: Data out-pipe.
	static const std::string m_oParamDelayPos;		///< Parameter: Amount of delay [um].

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

	/**
	 * @brief Arm the filter. The filter will empty
	 */
	virtual void arm(const fliplib::ArmStateBase& state);

protected:

	/**
	 * @brief In-pipe registration.
	 * @param p_rPipe Reference to pipe that is getting connected to the filter.
	 * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
	 */
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/**
	 * @brief Central signal processing routine.
	 * @param p_pSender
	 * @param p_rEvent
	 */
	void proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& e );

protected:

	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInData;			///< Data in-pipe.
	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInPos;			///< Position in-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOutData;			///< Data out-pipe.

	unsigned int												m_oDelayPos;				///< Parameter: Amount of delay [um].

	std::deque< interface::GeoDoublearray > 					m_oDataQueue;				///< The data queue.
	std::deque< interface::GeoDoublearray > 					m_oPosQueue;				///< The pos queue.	

}; // class Delay

} // namespace filter
} // namespace precitec

#endif // DELAY_POSDATA_H_
