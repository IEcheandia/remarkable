/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2011
 * 	@brief		Fliplib filter 'LineMovingAverage' in component 'Filter_LineGeometry'. 1d lowpass filter.
 */

#ifndef LINEMOVINGAVERAGE_H_
#define LINEMOVINGAVERAGE_H_

#include <memory>						///< unique_ptr

#include "fliplib/Fliplib.h"			///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"	///< base class
#include "fliplib/PipeEventArgs.h"		///< event processing
#include "fliplib/SynchronePipe.h"		///< in- / output

#include "geo/geo.h"					///< Size2d, VecDoublearray
#include "geo/array.h"					///< TArray
#include "filter/movingWindow.h"		///< sliding window filter
#include "filter/parameterEnums.h"		///< LowPassType


namespace precitec {
namespace filter {

/**
 * @ingroup Filter_LineGeometry
 * @brief This moving average filter averages a number of input samples and produce a single output sample.
 *
 * @details This averaging action removes the high frequency components present in the signal.
 * Moving average filters are normally used as low pass filters.
 * ATTENTION: Filter is causal, i.e. output depends only on the current and previous input values.
 * E.g. N = 3, f[x] = (f[x] + f[x-1] + f[x-2]) / 3
*/
class FILTER_API LineMovingAverage  : public fliplib::TransformFilter
{
public:
	LineMovingAverage();

	static const std::string m_oFilterName;			///< Name Filter
	static const std::string m_oPipeOutName;		///< Name Out-Pipe

	/// Set filter parameters as defined in database / xml file
	void setParameter();
	/// Paint overerlay primitives
	void paint();
	void arm(const fliplib::ArmStateBase& state);	///< arm filter

	/**
	 * @brief Calculates Moving Average.
	 * @details ATTENTION: Filter is causal, i.e. output depends only on the current and previous input values.
	 * E.g. N = 3, f[x] = (f[x] + f[x-1] + f[x-2]) / 3.
	 *
	 * @param p_rLineIn Input laser line. Contains y values at a certain x sampling rate. Ignores all values with bad rank (= RANK_MIN).
	 * @param p_oFilterLength Length of window average is calculated on (boxcar length).
	 * @param p_rLineOut Output laser line. May be identical to input (inplace processing).
	 * @return void
	 *
	 * @sa lineLowPass.h median1d.h
	 */
	static void lineMovingAverage(
			const geo2d::VecDoublearray&	p_rVecLineIn,
			geo2d::VecDoublearray&			p_rVecLineOut,
			MovingWindow<double>*			p_pMovingWindow);

protected:

	/// in pipe registrastion
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// in pipe event processing
	void proceed(const void* sender, fliplib::PipeEventArgs& e);

private:
	typedef fliplib::SynchronePipe< interface::GeoVecDoublearray >	SyncPipeGeoVecDoublearray;

	const SyncPipeGeoVecDoublearray				*m_pPipeInLine;				///< in pipe
	SyncPipeGeoVecDoublearray					m_oPipeOutLine;				///< out pipe

	interface::SmpTrafo							m_oSpTrafo;					///< roi translation
	geo2d::VecDoublearray						m_oLineOut;					///< output laser line
	unsigned int								m_oFilterRadius;			///< filter radius (filter lenght-1 / 2)
	unsigned int								m_oFilterLength;			///< filter length
	FilterAlgorithmType							m_oLowPassType;				///< type of low pass algorithm
	bool										m_oPassThroughBadRank;		///< if bad ranked values are always passed through and not eliminated

	std::unique_ptr<MovingWindow<double>>		m_oUpLowPass;				///< boxcar filter
	std::vector<double> m_oValuesInWindow;

}; // LineMovingAverage



} // namespace filter
} // namespace precitec

#endif /*LINEMOVINGAVERAGE_H_*/



