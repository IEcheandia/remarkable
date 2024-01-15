/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		Sk
 * 	@date		2015
 * 	@brief		Result filter for double values. Performs a range check of the value given from external.
 */

#ifndef EXTRANGECHECK_H_
#define EXTRANGECHECK_H_

#include "fliplib/Fliplib.h"
#include "fliplib/ResultFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "event/results.h"
#include "geo/range.h"

namespace precitec {
namespace filter {

class FILTER_API ExtRangeCheck : public fliplib::ResultFilter
{
public:
	ExtRangeCheck();

	static const std::string m_oFilterName;
	static const std::string m_oPipeNameResult;
	static const std::string m_oResultType;            // User defined Resulttyp
	static const std::string m_oNioType;               // User defined Niotyp

	void setParameter();

protected:
	// protected methods

	bool subscribe(fliplib::BasePipe&, int);
	void proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& e );

	typedef fliplib::SynchronePipe<interface::GeoDoublearray>		pipe_scalar_t;
	typedef fliplib::SynchronePipe<interface::ResultDoubleArray>	pipe_result_t;

private:
	// private variables

	/* ------------- pipes ------------ */
	const pipe_scalar_t		*m_pPipeInDouble;		///< In-Pipe scalar
	const pipe_scalar_t		*m_pPipeInDoubleMax;	///< In-Pipe Max scalar
	const pipe_scalar_t		*m_pPipeInDoubleMin;	///< In-Pipe Min scalar	
	pipe_result_t			m_oPipeResultDouble;	///< Result-Pipe scalar

	/* ---- other member variables ---- */
	geo2d::Range1d			m_oMinMaxRange;		///< lower and upper limit for NIO/IO
	interface::ResultType	m_UserResultType;	///< User defined result type
	interface::ResultType	m_UserNioType;		///< User defined nio type
};

}
}

#endif /* EXTRANGECHECK_H_ */
