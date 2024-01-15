/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		Filter that produces a result when the current seam ends. The result filter will only produce a single result, will not send a result with every frame.
 *  The filter listens on the input pipe and stores the value of a specific frame internally. When the seam ends, it will send out this value as a result.
 */

#ifndef SEAMENDRESULT_H_
#define SEAMENDRESULT_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/ResultFilter.h"
#include "fliplib/SynchronePipe.h"
#include "common/frame.h"
#include "event/results.h"

namespace precitec {
namespace filter {

/**
 * 	@brief		Filter that produces a result when the current seam ends. The result filter will only produce a single result, will not send a result with every frame.
 *  The filter listens on the input pipe and stores the value that comes in with a specific frame number. When the seam ends, it will send out this value as a result.
 */
class FILTER_API SeamEndResult : public fliplib::ResultFilter
{
public:

	/**
	 * @brief CTor.
	 */
	SeamEndResult();
	/**
	 * @brief DTor.
	 */
	virtual ~SeamEndResult();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeResultName;		///< Pipe: Result out-pipe.
	static const std::string m_oParamFrame;			///< Parameter: Incoming value with this frame number needs to be kept and written out at end-of-seam.
	static const std::string m_oParamResultType;	///< Parameter: User-defined result type.
	static const std::string m_oParamNioType;		///< Parameter: User-defined nio type.

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

	/**
	 * @brief Arm the filter. Here, the filter sends out the result, if the state is eSeamEnd.
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
	fliplib::SynchronePipe<interface::ResultDoubleArray> 		m_oPipeResult;			///< Result out-pipe.

	int 														m_oFrame;				///< Incoming value with this frame number needs to be kept and written out at end-of-seam.
	int															m_oCounter;				///< Internal frame counter.
	interface::ResultDoubleArray								m_oOutValue;			///< The actual value that will be written out later.

	interface::ResultType										m_oUserResultType;		///< User defined result type.
	interface::ResultType										m_oUserNioType;			///< User defined nio type.

}; // class SeamEndResult

} // namespace filter
} // namespace precitec

#endif /* SeamEndResult_H_ */
