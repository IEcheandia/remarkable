/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2011
 * 	@brief		Fliplib filter 'TemporalLowPass' in component 'Filter_Utility'. Temporal lowpass filter for positions.
 */

#ifndef TEMPORALLOWPASS_H_20111028_H_
#define TEMPORALLOWPASS_H_20111028_H_

#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output

#include "temporalLowPassTemplate.h"

namespace precitec {
namespace filter {

/**
 * @ingroup Filter_Utility
 * @brief This temporal low pass filter averages over past input values and produces a single output value.
 *
 * @details This action removes the high frequency components present in the signal.
*/
class FILTER_API TemporalLowPass  : public fliplib::TransformFilter
{
public:
	TemporalLowPass();

	/// Set filter parameters as defined in database / xml file
	void setParameter();
private:
	typedef fliplib::SynchronePipe< interface::GeoDoublearray >	pipe_scalar_t;

	static const std::string m_oFilterName;			///< Name Filter
	static const std::string m_oPipeOutName0;		///< Name Out-Pipe


	/// arm filter
	/*virtual*/ void arm (const fliplib::ArmStateBase& p_rArmtate);

	/// in pipe registrastion
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// in pipe event processing
	void proceed(const void* sender, fliplib::PipeEventArgs& e);

	const pipe_scalar_t					*m_pPipeInGeoDoubleArray;	///< in pipe
	pipe_scalar_t						m_oPipeOutDouble;			///< out pipe

    TemporalLowPassTemplate<1> m_calculator;

}; // TemporalLowPass


} // namespace filter
} // namespace precitec 

#endif /*TEMPORALLOWPASS_H_20111028_H_*/



