/*!
 *  @file	
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS), Stefan Birmanns (SB)
 *  @date			2012	
 *	@brief			Sample source filter that emits a 1d data vector.
 *	@details		PLEASE NOTE: This code is identical for the four filters 'SampleSource', 'HeadMonitorSource', 'EncoderPositionSource' and 'LaserPowerSource'.
 */

#ifndef SAMPLESOURCE_H_
#define SAMPLESOURCE_H_

#include "fliplib/Fliplib.h"
#include "fliplib/SourceFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "common/frame.h"
#include "geo/geo.h"

namespace precitec {
namespace filter {

class FILTER_API SampleSource : public fliplib::SourceFilter
{
public:
	SampleSource();

	static const std::string m_oFilterName;
	static const std::string PIPENAME_SAMPLEOUT;

protected:
	void setParameter();
	bool subscribe(fliplib::BasePipe&, int);
	void proceed(const void*, fliplib::PipeEventArgs&);

private:
	const fliplib::SynchronePipe< interface::SampleFrame >	*m_pSampleIn;		///< In Connector,  SampleFrame
	fliplib::SynchronePipe< interface::GeoDoublearray >		m_oPipeOut;			///< Out Connector

	int														m_oDefaultValue;	///< default value used for simulation
};

} // namespace filter
} // namespace precitec

#endif /*SAMPLESOURCE_H_*/


