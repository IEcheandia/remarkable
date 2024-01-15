/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		SB
 * 	@date		2017
 * 	@brief		A filter that checks if the seam was welded long enough. The actual detection of the seam has to be provided by other filter, this filter here only sums up the
 * 				length via the images in which the seam was detected and compares that with the length of the seam as it was specified in the product tree.
 */

#ifndef NOSEAMCHECK_H_
#define NOSEAMCHECK_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/ResultFilter.h"
#include "fliplib/SynchronePipe.h"
#include "common/frame.h"
#include "event/results.h"

namespace precitec {
namespace filter {

/**
 * 	@brief		A filter that checks if the seam was welded long enough. The actual detection of the seam has to be provided by other filter, this filter here only sums up the
 * 				length via the images in which the seam was detected and compares that with the length of the seam as it was specified in the product tree.
 */
class FILTER_API NoSeamCheck : public fliplib::ResultFilter
{
public:

	/**
	 * @brief CTor.
	 */
	NoSeamCheck();
	/**
	 * @brief DTor.
	 */
	virtual ~NoSeamCheck();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeResultName;		///< Pipe: Result out-pipe.
	static const std::string m_oParamTolerance;		///< Tolerated length deviation in percent.
	static const std::string m_oParamInverse;		///< One can also use the filter to test if an error is present for a certain length of the seam.
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
	void proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent );

protected:

	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInData;			///< Data in-pipe.
	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInSpeed;			///< Robot-speed in-pipe.
	fliplib::SynchronePipe<interface::ResultDoubleArray> 		m_oPipeResult;			///< Result out-pipe.

	double														m_oTriggerFreq;			///< Trigger delta in [ms]!
	double														m_oTolerance;			///< Tolerated length deviation in percent.
	bool														m_oInverse;				///< Inverse check: If the accumulated value is higher than the tolerance times seam length -> error.
	double														m_oCurrLength;			///< Current length.
	double														m_oSeamLength;			///< Total length of the seam, as provided by the product data structure.

	interface::ResultDoubleArray								m_oOutValue;			///< The actual value that will be written out later.

	interface::ResultType										m_oUserResultType;		///< User defined result type.
	interface::ResultType										m_oUserNioType;			///< User defined nio type.

}; // class NoSeamCheck

} // namespace filter
} // namespace precitec

#endif /* NOSEAMCHECK_H_ */
