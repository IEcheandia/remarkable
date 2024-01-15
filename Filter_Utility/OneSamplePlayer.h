/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter retrieves data elements from a global a buffer. The data elements were stored in the buffer by the BufferRecorder filter.
 */

#ifndef ONESAMPLEPLAYER_H_
#define ONESAMPLEPLAYER_H_

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
class FILTER_API OneSamplePlayer : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	OneSamplePlayer();
	/**
	 * @brief DTor.
	 */
	virtual ~OneSamplePlayer();

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
	void proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rEvent );

	int m_oSlotNumber;
	int m_oCurrentNumber;

protected:

	const fliplib::SynchronePipe< interface::ImageFrame >*		m_pPipeInImage;			///< Data in-pipe - image.
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOutData;			///< Data out-pipe.


}; // class BufferPlayer


} // namespace filter
} // namespace precitec

#endif /* ONESAMPLEPLAYER_H_ */
