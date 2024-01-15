/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2017
 * 	@brief		This filter will only send a valid result out, if a second pipe delivers a value over a certain threshold. Otherwise the result will be declared invalid and then send out - which will cause the sum-errors to ignore it.
 */

#ifndef CONDITIONALRESULT_H_
#define CONDITIONALRESULT_H_

#include "fliplib/Fliplib.h"
#include "fliplib/ResultFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "event/results.h"
//#include "geo/range.h"

namespace precitec {
namespace filter {

/**
 * @brief This filter will only send a valid result out, if a second pipe delivers a value over a certain threshold. Otherwise the result will be declared invalid and then send out - which will cause the sum-errors to ignore it.
 */
class FILTER_API ConditionalResult : public fliplib::ResultFilter
{
public:

	/**
	 * @brief CTor.
	 */
	ConditionalResult();
	/**
	 * @brief DTor.
	 */
	virtual ~ConditionalResult();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeResultName;		///< Pipe: Result out-pipe.
	static const std::string m_oParamThreshold;		///< Parameter: Threshold below which the result is declared as not valid.
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
	bool subscribe(fliplib::BasePipe&, int);
	/**
	 * @brief Processing routine.
	 */
	void proceedGroup(const void*, fliplib::PipeGroupEventArgs&);

	typedef fliplib::SynchronePipe<interface::GeoDoublearray>		pipe_scalar_t;
	typedef fliplib::SynchronePipe<interface::ResultDoubleArray>	pipe_result_t;

private:

	const pipe_scalar_t*	m_pPipeInDouble;		///< In-Pipe with the actual measurement value.
	const pipe_scalar_t*	m_pPipeInValid;			///< In-Pipe with the information about the validity of the measurement.
	pipe_result_t			m_oPipeResult;			///< Result-Pipe scalar

	double					m_oThreshold;			///< Threshold above which the result is declared valid.

	interface::ResultType	m_oUserResultType;		///< User defined result type
	interface::ResultType	m_oUserNioType;			///< User defined nio type

};

} // namespace filter
} // namespace precitec

#endif /* CONDITIONALRESULT_H_ */
