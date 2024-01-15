/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		AB, HS
 * 	@date		2011
 * 	@brief		Result filter for double values. Performs a range check of the value.
 */

#ifndef RANGECHECK_H_
#define RANGECHECK_H_

#include "fliplib/Fliplib.h"
#include "fliplib/ResultFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "event/results.h"
#include "geo/range.h"

namespace precitec {
namespace filter {

class FILTER_API RangeCheck : public fliplib::ResultFilter
{
public:
	RangeCheck();

	static const std::string m_oFilterName;
	static const std::string m_oPipeNameResult;
	static const std::string m_oResultType;            // User defined Resulttyp
	static const std::string m_oNioType;               // User defined Niotyp

	void setParameter();

protected:
	// protected methods

	bool subscribe(fliplib::BasePipe&, int);
	void proceed(const void*, fliplib::PipeEventArgs&);

	typedef fliplib::SynchronePipe<interface::GeoDoublearray>		pipe_scalar_t;
	typedef fliplib::SynchronePipe<interface::ResultDoubleArray>	pipe_result_t;

private:
	// private variables

	/* ------------- pipes ------------ */
	const pipe_scalar_t		*m_pPipeInDouble;		///< In-Pipe scalar
	pipe_result_t			m_oPipeResultDouble;	///< Result-Pipe scalar

	/* ---- other member variables ---- */
	/// boundaries, to be set in setParameter
	geo2d::Range1d			m_oMinMaxRange;		///< lower and upper limit for NIO/IO
	interface::ResultType	m_UserResultType;	///< User defined result type
	interface::ResultType	m_UserNioType;		///< User defined nio type
};

}
}

#endif /* RF3DCOORDLIMITS_H_ */
