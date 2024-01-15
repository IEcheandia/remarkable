/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		Filter that checks if image contains a signal and sends an NIO if not.
 */

#ifndef BLACKIMAGECHECK_H_
#define BLACKIMAGECHECK_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/ResultFilter.h"
#include "fliplib/SynchronePipe.h"
#include "common/frame.h"
#include "event/results.h"

namespace precitec {
namespace filter {

/**
 * @brief Filter that checks if image contains a signal and sends an NIO if not.
 */
class FILTER_API BlackImageCheck : public fliplib::ResultFilter
{
public:

	/**
	 * @brief CTor.
	 */
	BlackImageCheck();
	/**
	 * @brief DTor.
	 */
	virtual ~BlackImageCheck();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeResultName;		///< Pipe: Result out-pipe.
	static const std::string m_oParamThreshold;		///< Parameter: Threshold below which a pixel is not part of the signal.
	static const std::string m_oParamResultType;	///< Parameter: User-defined result type.
	static const std::string m_oParamNioType;		///< Parameter: User-defined nio type.

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
	void proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rEvent );

protected:

	const fliplib::SynchronePipe< interface::ImageFrame >*		m_pPipeInImageFrame;	///< Image in-pipe.
	fliplib::SynchronePipe<interface::ResultDoubleArray> 		m_oPipeResult;			///< Result out-pipe.

	int 														m_oThreshold;			///< Threshold value, below which a pixel does not contain signal.

	interface::ResultType										m_oUserResultType;		///< User defined result type.
	interface::ResultType										m_oUserNioType;			///< User defined nio type.

}; // class BlackImageCheck

} // namespace filter
} // namespace precitec

#endif /* BLACKIMAGECHECK_H_ */
