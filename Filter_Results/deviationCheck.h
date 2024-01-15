/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		AB, HS
 * 	@date		2011
 * 	@brief		Result filter for double values. Performs a range check of the value.
 */

#ifndef DEVIATIONCHECK_H_
#define DEVIATIONCHECK_H_

#include "fliplib/Fliplib.h"
#include "fliplib/ResultFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "event/results.h"
#include "geo/range.h"

namespace precitec {
namespace filter {

class FILTER_API DeviationCheck : public fliplib::ResultFilter
{
public:
	DeviationCheck();
	virtual ~DeviationCheck();

	static const std::string m_oFilterName;
	static const std::string m_oPipeNameResult;
	static const std::string m_oResultType;
	static const std::string m_oNioType;

	void setParameter();

protected:
	// protected methods

	bool subscribe(fliplib::BasePipe&, int);
	void proceed(const void*, fliplib::PipeEventArgs&);

private:
	// private variables

	// pipes
	const fliplib::SynchronePipe<interface::GeoDoublearray>*	m_pPipeInDouble;		///< In-Pipe scalar
	fliplib::SynchronePipe<interface::ResultDoubleArray>		m_oPipeResultDouble;	///< Result-Pipe double scalar

	// parameters
	double					m_oReferenceValue;  ///< Base value
	double					m_oPercent;			///< Percentage of max. deviation allowed
	bool					m_oAbove;			///< Allow deviation above base value 
	bool					m_oBelow;			///< Allow deviation below base value
	
	// internal variables
	geo2d::TRange<double>	m_oMinMaxRange;		///< lower and upper limit for NIO/IO

	// user defined values
	interface::ResultType	m_UserResultType;	///< User defined result type
	interface::ResultType	m_UserNioType;		///< User defined nio type
};

}
}

#endif /* RF3DCOORDLIMITS_H_ */
