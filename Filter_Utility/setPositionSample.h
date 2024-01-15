/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter retrieves data elements from a global a buffer. The data elements were stored in the buffer by the BufferRecorder filter.
 */

#ifndef SETPOSITIONSAMPLE_H_
#define SETPOSITIONSAMPLE_H_

// WM includes
#include <fliplib/Fliplib.h>
#include <fliplib/TransformFilter.h>
#include <fliplib/SynchronePipe.h>
#include <geo/geo.h>
#include "common/frame.h"

namespace precitec {
namespace filter {

/**
 * @brief This filter retrieves data elements from a global a buffer. The data elements were stored in the buffer by the BufferRecorder filter.
 */
class FILTER_API SetPositionSample : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	SetPositionSample();
	/**
	 * @brief DTor.
	 */
	virtual ~SetPositionSample();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeOutName;		///< Pipe: Data out-pipe.

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

	/**
	 * @brief Arm the filter. This means here, that the length of the seam is determined and memory is allocated for all the data elements.
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
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

protected:

	const fliplib::SynchronePipe< interface::GeoDoublearray >*		m_pPipeInSample;			///< Data in-pipe - image.
	const fliplib::SynchronePipe< interface::GeoDoublearray >*		m_pPipeInValue;			///< Data in-pipe - value.
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOutSample;			///< Data out-pipe.


}; // class BufferPlayer


} // namespace filter
} // namespace precitec

#endif /* ONESAMPLEPLAYER_H_ */
