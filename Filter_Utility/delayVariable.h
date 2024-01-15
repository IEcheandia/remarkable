/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter delays the output of data elements, but compared to the normal delay filter, this one has a separate in-pipe for the velocity.
 *              This means the delay is now a variable.
 */

#ifndef DELAY_VELOCITY_H_
#define DELAY_VELOCITY_H_

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
class FILTER_API DelayVariable : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	DelayVariable();
	/**
	 * @brief DTor.
	 */
	virtual ~DelayVariable();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeOutName;		///< Pipe: Data out-pipe.
	static const std::string m_oParamDelay;			///< Parameter: Amount of delay [um].
	static const std::string m_oParamFill;			///< Parameter: How is the initial gap supposed to be filled?

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
	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInVelocity;		///< Velocity in-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOutData;			///< Data out-pipe.

	unsigned int												m_oDelay;				///< Parameter: Amount of delay [um].
	unsigned int 												m_oFill;				///< Parameter: How is the initial gap supposed to be filled?

	int 														m_oTriggerFreq;			///< Trigger frequency [s]

	std::deque< interface::GeoDoublearray > 					m_oQueue;				///< The data queue.
    Poco::FastMutex m_oMutex;

}; // class Delay

} // namespace filter
} // namespace precitec

#endif // DELAY_H_
