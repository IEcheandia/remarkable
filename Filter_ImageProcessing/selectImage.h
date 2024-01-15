/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		DUW
 * 	@date		2015
 * 	@brief		This filter may discard images or select them for processing given a delay.
 */

#ifndef SELECT_IMAGE_H_
#define SELECT_IMAGE_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"

#include "common/frame.h"   // fuer ImageFrame

namespace precitec {
namespace filter {

/**
 * @brief This filter delays the output of data elements.
 */
class FILTER_API SelectImage : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	SelectImage();
	/**
	 * @brief DTor.
	 */
	virtual ~SelectImage();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeOutName;		///< Pipe: Data out-pipe.
	static const std::string m_oParamSelectThresholdName;	///< Parameter: Amount of delay [um].
	static const std::string m_oParamEndThresholdName;	///< Parameter: when to stop processing [um].

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

	typedef fliplib::SynchronePipe< interface::ImageFrame >	SyncPipeImgFrame;
	const SyncPipeImgFrame*	m_pPipeInImageFrame;	///< inpipe
	SyncPipeImgFrame		m_oPipeOutImageFrame;	///< outpipe 

	unsigned int												m_oSelectThreshold;				///< Parameter: Amount of delay [um].
	int 														m_oTriggerDelta;		///< Trigger distance [um]
	unsigned int												m_oEndThreshold;				///< Parameter: Amount of delay [um].
}; // class Delay

} // namespace filter
} // namespace precitec

#endif // SELECT_IMAGE_H_
