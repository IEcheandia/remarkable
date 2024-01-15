/*!
 *  @file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS), Stefan Birmanns (SB)
 *  @date			2012
 *	@brief			Sample source filter that emits a 1d data vector.
 *	@details		PLEASE NOTE: This code is identical for the four filters 'LaserPowerSource', 'HeadMonitorSource', 'EncoderPositionSource' and 'LaserPowerSource'.
 */

#include "laserPowerSource.h"

#include "event/sensor.h"
#include "filter/sensorFilterInterface.h"
#include "geo/array.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace image;
	using namespace interface;

namespace filter {

const std::string LaserPowerSource::m_oFilterName 		= std::string("LaserPowerSource");
const std::string LaserPowerSource::PIPENAME_SAMPLEOUT	= std::string("SampleFrame");

LaserPowerSource::LaserPowerSource() : SourceFilter( LaserPowerSource::m_oFilterName, Poco::UUID{"75A0CEDF-C3E8-458A-9CD4-C15058BAD30B"}, eExternSensorDefault ),
	m_pSampleIn			( nullptr ),
	m_oPipeOut			( this, LaserPowerSource::PIPENAME_SAMPLEOUT ),
	m_oDefaultValue		( 0 )
{
	// Defaultwerte der Parameter setzen

	parameters_.add("DefaultValue",		Parameter::TYPE_int, m_oDefaultValue);

    setOutPipeConnectors({{Poco::UUID("C4436C55-7197-4B4D-9F15-B1827E059835"), &m_oPipeOut, PIPENAME_SAMPLEOUT, 0, ""}});
    setVariantID(Poco::UUID("9C4FA73C-75C0-40D8-968D-C5D491C306AA"));
}



void LaserPowerSource::setParameter() {
	SourceFilter::setParameter();
	m_oDefaultValue = parameters_.getParameter("DefaultValue").convert<int>();
}



bool LaserPowerSource::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.name() == precitec::interface::SensorFilterInterface::SENSOR_SAMPLE_FRAME_PIPE)
	{
		m_pSampleIn  = dynamic_cast< fliplib::SynchronePipe<interface::SampleFrame>* >(&p_rPipe);
		return BaseFilter::subscribe( p_rPipe, p_oGroup );
	}
	return false;
}



void LaserPowerSource::proceed(const void* p_pSender, PipeEventArgs& p_rEventArgs)
{
	poco_assert_dbg(m_pSampleIn != nullptr); // to be asserted by graph editor

	const interface::SampleFrame	&rSampleFrameIn = m_pSampleIn->read(m_oCounter);
	const image::Sample				&rSample(rSampleFrameIn.data());

	geo2d::Doublearray			oDoublearray	(rSample.numElements(), 0.0, eRankMax);
	auto						&rData			(oDoublearray.getData());
	for ( int i = 0; i < rSample.numElements(); i++ )
	{
		if ( rSample[i] >= 0 )
			rData[i]	= double(rSample[i]) / 3276.70;
		else
			rData[i]	= double(rSample[i]) / 3276.80;

	} // for

	const interface::GeoDoublearray		oGeoDoublearrayOut(rSampleFrameIn.context(), oDoublearray, rSampleFrameIn.analysisResult(), interface::Limit);
	preSignalAction(); m_oPipeOut.signal(oGeoDoublearrayOut);
}


} // namespace filter
} // namespace precitec

