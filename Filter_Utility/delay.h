/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter delays the output of data elements.
 */

#ifndef DELAY_H_
#define DELAY_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>
// Poco includes
#include "Poco/Mutex.h"
// stl includes
#include <queue>

namespace precitec {
namespace filter {

/**
 * @brief This filter delays the output of data elements.
 */
class FILTER_API Delay : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	Delay();
	/**
	 * @brief DTor.
	 */
	virtual ~Delay();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeOutName;		///< Pipe: Data out-pipe.
	static const std::string m_oParamDelay;			///< Parameter: Amount of delay [um].
	static const std::string m_oParamFill;			///< Parameter: How is the initial gap supposed to be filled?

    enum class eFillType
    {
        DoNotFill = 0,
        Zeros = 1,
        FillWithFirst =2
    };

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
	 * @brief Processing routine.
	 * @param p_pSender pointer to
	 * @param p_rEvent
	 */
	void proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rEvent );

protected:

	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInData;			///< Data in-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOutData;			///< Data out-pipe.

	unsigned int												m_oDelay;				///< Parameter: Amount of delay [um].
    eFillType 													m_oFill;				///< Parameter: How is the initial gap supposed to be filled?

	int 														m_oTriggerDelta;		///< Trigger distance [um]

	std::queue< interface::GeoDoublearray > 					m_oQueue;				///< The data queue.
    Poco::FastMutex m_oMutex;

}; // class Delay

} // namespace filter
} // namespace precitec

#endif // DELAY_H_
