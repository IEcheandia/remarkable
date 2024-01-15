/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		AL
 * 	@date		2016
 * 	@brief		Line profile result filter.
 */

#ifndef LINEPROFILERESULT_H_
#define LINEPROFILERESULT_H_

#include "fliplib/Fliplib.h"
#include "fliplib/ResultFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "common/frame.h"
#include "event/results.h"
#include "geo/range.h"

namespace precitec {
namespace filter {

class FILTER_API LineProfileResult : public fliplib::ResultFilter
{
public:
	LineProfileResult();

	static const std::string m_oFilterName;
	static const std::string m_oPipeNameResult;
	static const std::string m_oResultTypeParameter;
	
	void setParameter();

protected:
	// protected methods

	bool subscribe(fliplib::BasePipe&, int);
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArgs);

	typedef fliplib::SynchronePipe<interface::GeoDoublearray>		pipe_scalar_t;
	typedef fliplib::SynchronePipe<interface::ResultDoubleArray>	pipe_result_t;

private:
	// private variables

	/* ------------- pipes ------------ */
	typedef fliplib::SynchronePipe< interface::ImageFrame >	SyncPipeImgFrame;
	
	const fliplib::SynchronePipe< interface::GeoVecDoublearray >*		m_pPipeLineIn;		///< In-Pipe line
	const SyncPipeImgFrame*	m_pPipeInImageFrame;	///< inpipe

	pipe_result_t			m_oPipeResultDouble;	///< Result-Pipe scalar

	/* ---- other member variables ---- */
	/// boundaries, to be set in setParameter
	geo2d::Range1d			m_oMinMaxRange;		///< lower and upper limit for NIO/IO
	interface::SmpTrafo		m_oSpTrafo;				///< roi translation
	int	m_oResultType;	///< User defined result type
};

}
}

#endif /* LINEPROFILERESULT_H_ */
