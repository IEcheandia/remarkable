/*!
 *  @file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS), Stefan Birmanns (SB)
 *  @date			2012
 *	@brief			Sample source filter that emits a 1d data vector.
 *	@details		PLEASE NOTE: This code is identical for the four filters 'HeadMonitorSource', 'HeadMonitorSource', 'EncoderPositionSource' and 'LaserPowerSource'.
 */

#include "headMonitorSource.h"

#include "event/sensor.h"
#include "filter/sensorFilterInterface.h"
#include "geo/array.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace image;
	using namespace interface;

namespace filter {

const std::string HeadMonitorSource::m_oFilterName 		= std::string("HeadMonitorSource");
const std::string HeadMonitorSource::PIPENAME_SAMPLEOUT	= std::string("SampleFrame");

HeadMonitorSource::HeadMonitorSource() : SourceFilter( HeadMonitorSource::m_oFilterName, Poco::UUID{"7EE3EBA0-1803-45D5-AE01-2DECC9E5992E"}, eExternSensorDefault ),
	m_pSampleIn			( nullptr ),
	m_oPipeOut			( this, HeadMonitorSource::PIPENAME_SAMPLEOUT ),
	m_oDefaultValue		( 0 )
{
	// Defaultwerte der Parameter setzen

	parameters_.add("DefaultValue",		Parameter::TYPE_int, m_oDefaultValue);

    setOutPipeConnectors({{Poco::UUID("0B7FBBB8-8777-4486-A280-F4D044C6021E"), &m_oPipeOut, PIPENAME_SAMPLEOUT, 0, ""}});
    setVariantID(Poco::UUID("388A40C9-C79D-4DF4-9CF5-08DBAFAF2C4B"));
}



void HeadMonitorSource::setParameter() {
	SourceFilter::setParameter();
	m_oDefaultValue = parameters_.getParameter("DefaultValue").convert<int>();
}



bool HeadMonitorSource::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.name() == precitec::interface::SensorFilterInterface::SENSOR_SAMPLE_FRAME_PIPE)
	{
		m_pSampleIn  = dynamic_cast< fliplib::SynchronePipe<interface::SampleFrame>* >(&p_rPipe);
		return BaseFilter::subscribe( p_rPipe, p_oGroup );
	}
	return false;
}



void HeadMonitorSource::proceed(const void* p_pSender, PipeEventArgs& p_rEventArgs)
{
	poco_assert_dbg(m_pSampleIn != nullptr); // to be asserted by graph editor


	const interface::SampleFrame	&rSampleFrameIn = m_pSampleIn->read(m_oCounter);
	const image::Sample				&rSample(rSampleFrameIn.data());

	geo2d::Doublearray			oDoublearray	(rSample.numElements(), 0.0, eRankMax);
	auto						&rData			(oDoublearray.getData());
	for (int i = 0; i < rSample.numElements(); i++) {
		rData[i]	= rSample[i];
	} // for

	const interface::GeoDoublearray		oGeoDoublearrayOut(rSampleFrameIn.context(), oDoublearray, rSampleFrameIn.analysisResult(), interface::Limit);
	preSignalAction(); m_oPipeOut.signal(oGeoDoublearrayOut);
}


} // namespace filter
} // namespace precitec

