/*!
 *  @file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS), Stefan Birmanns (SB)
 *  @date			2012
 *	@brief			Sample source filter that emits a 1d data vector.
 *	@details		PLEASE NOTE: This code is identical for the four filters 'EncoderPositionSource', 'HeadMonitorSource', 'EncoderPositionSource' and 'LaserPowerSource'.
 */

#include "encoderPositionSource.h"

#include "event/sensor.h"
#include "filter/sensorFilterInterface.h"
#include "geo/array.h"
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace image;
	using namespace interface;

namespace filter {

const std::string EncoderPositionSource::m_oFilterName 		= std::string("EncoderPositionSource");
const std::string EncoderPositionSource::PIPENAME_SAMPLEOUT	= std::string("SampleFrame");

EncoderPositionSource::EncoderPositionSource() : SourceFilter( EncoderPositionSource::m_oFilterName, Poco::UUID{"9FF0BE12-300F-40D7-965D-3FC300519A08"}, eExternSensorDefault ),
	m_pSampleIn			( nullptr ),
	m_oPipeOut			( this, EncoderPositionSource::PIPENAME_SAMPLEOUT ),
	m_oDefaultValue		( 0 )
{
	// Defaultwerte der Parameter setzen

	parameters_.add("DefaultValue",		Parameter::TYPE_int, m_oDefaultValue);

    setOutPipeConnectors({{Poco::UUID("80B5687D-D1AA-4D97-9D5E-A5B05D25BF8A"), &m_oPipeOut, PIPENAME_SAMPLEOUT, 0, ""}});
    setVariantID(Poco::UUID("F9E06777-4405-491B-BEC7-32DFB938CE7B"));
}



void EncoderPositionSource::setParameter() {
	SourceFilter::setParameter();
	m_oDefaultValue = parameters_.getParameter("DefaultValue").convert<int>();
}



bool EncoderPositionSource::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.name() == precitec::interface::SensorFilterInterface::SENSOR_SAMPLE_FRAME_PIPE)
	{
		m_pSampleIn  = dynamic_cast< fliplib::SynchronePipe<interface::SampleFrame>* >(&p_rPipe);
		return BaseFilter::subscribe( p_rPipe, p_oGroup );
	}
	return false;
}



void EncoderPositionSource::proceed(const void* p_pSender, PipeEventArgs& p_rEventArgs)
{
	poco_assert_dbg(m_pSampleIn != nullptr); // to be asserted by graph editor


	const interface::SampleFrame	&rSampleFrameIn = m_pSampleIn->read(m_oCounter);
	const image::Sample				&rSample(rSampleFrameIn.data());

	geo2d::Doublearray			oDoublearray	(rSample.numElements(), 0.0, eRankMax);
	auto						&rData			(oDoublearray.getData());
	for (int i = 0; i < rSample.numElements(); i++)
	{
		rData[i]	= rSample[i];
		if(m_oVerbosity >= eHigh)
			wmLog( eDebug, "EncoderPositionSource::proceed: %f\n", rData[i] );
	} // for

	const interface::GeoDoublearray		oGeoDoublearrayOut(rSampleFrameIn.context(), oDoublearray, rSampleFrameIn.analysisResult(), interface::Limit);
	preSignalAction(); m_oPipeOut.signal(oGeoDoublearrayOut);
}


} // namespace filter
} // namespace precitec

