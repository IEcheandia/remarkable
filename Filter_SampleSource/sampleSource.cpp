/*!
 *  @file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS), Stefan Birmanns (SB)
 *  @date			2012
 *	@brief			Sample source filter that emits a 1d data vector.
 *	@details		PLEASE NOTE: This code is identical for the four filters 'SampleSource', 'HeadMonitorSource', 'EncoderPositionSource' and 'LaserPowerSource'.
 */

#include "sampleSource.h"

#include "event/sensor.h"
#include "filter/sensorFilterInterface.h"
#include <fliplib/TypeToDataTypeImpl.h>
#include "geo/array.h"
#include "util/calibDataSingleton.h"
#include "common/definesIDM.h"
#include "common/systemConfiguration.h"

using namespace fliplib;
namespace precitec {
	using namespace image;
	using namespace interface;

namespace filter {

const std::string SampleSource::m_oFilterName 		= std::string("SampleSource");
const std::string SampleSource::PIPENAME_SAMPLEOUT	= std::string("SampleFrame");

SampleSource::SampleSource() : SourceFilter( SampleSource::m_oFilterName, Poco::UUID{"E2FF0EC4-D2CE-47C3-8C92-9E2B1294C8EA"}, eExternSensorDefault ),
	m_pSampleIn			( nullptr ),
	m_oPipeOut			( this, SampleSource::PIPENAME_SAMPLEOUT ),
	m_oDefaultValue		( 0 )
{
	// Defaultwerte der Parameter setzen

	parameters_.add("DefaultValue",		Parameter::TYPE_int, m_oDefaultValue);

    setOutPipeConnectors({{Poco::UUID("bf863352-f08c-46f8-a356-7aaa88af0bc0"), &m_oPipeOut, PIPENAME_SAMPLEOUT, 0, ""}});
    setVariantID(Poco::UUID("7005FFB6-246B-495d-BE40-A50FCCDF5E91"));
}



void SampleSource::setParameter() {
	SourceFilter::setParameter();
	m_oDefaultValue = parameters_.getParameter("DefaultValue").convert<int>();
}



bool SampleSource::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.name() == precitec::interface::SensorFilterInterface::SENSOR_SAMPLE_FRAME_PIPE)
	{
		m_pSampleIn  = dynamic_cast< fliplib::SynchronePipe<interface::SampleFrame>* >(&p_rPipe);
		return BaseFilter::subscribe( p_rPipe, p_oGroup );
	}
	return false;
}



void SampleSource::proceed(const void* p_pSender, PipeEventArgs& p_rEventArgs)
{
	poco_assert_dbg(m_pSampleIn != nullptr); // to be asserted by graph editor

	const interface::SampleFrame	&rSampleFrameIn = m_pSampleIn->read(m_oCounter);
	const image::Sample				&rSample(rSampleFrameIn.data());

	geo2d::Doublearray			oDoublearray	(rSample.numElements(), 0.0, eRankMax);
	auto						&rData			(oDoublearray.getData());
	for (int i = 0; i < rSample.numElements(); i++)
    {
		rData[i]	= rSample[i];
	} // for

	//correct IDMWeldingDepth in the output of the sample source filter
	if (getSensorID() == eIDMWeldingDepth)
    {
        auto &rCalibData = precitec::system::CalibDataSingleton::getCalibrationDataReference(precitec::math::SensorId::eSensorId0);
        auto &rScannerContext = rSampleFrameIn.context().m_ScannerInfo;
        const auto hasFiberSwitch = SystemConfiguration::instance().getBool("OCT_with_reference_arms", false);
        if (rCalibData.hasIDMCorrectionGrid() && rScannerContext.m_hasPosition && !hasFiberSwitch)
        {
            for (int i = 0; i < rSample.numElements(); i++)
            {
                if (rSample[i] != IDMWELDINGDEPTH_BADVALUE)
                {
                    rData[i]	= rCalibData.getCalibratedIDMWeldingDepth(rScannerContext.m_x , rScannerContext.m_y, rSample[i]);
                }
                else
                {
                    rData[i] = IDMWELDINGDEPTH_BADVALUE;
                    oDoublearray.getRank()[i] = eRankMin;
                }
            } // for
        }
        else
        {
            // no idm correction to apply, but we still need to check bad measurements
            for (int i = 0; i < rSample.numElements(); i++)
            {
                if (rSample[i] == IDMWELDINGDEPTH_BADVALUE)
                {
                    rData[i] = IDMWELDINGDEPTH_BADVALUE;
                    oDoublearray.getRank()[i] = eRankMin;
                }
            } // for
        }
        if (m_oVerbosity >= VerbosityType::eHigh)
        {
            if (!rCalibData.hasIDMCorrectionGrid())
            {
                wmLog(eDebug, "No IDM Correction grid \n");
            }
            if (!rScannerContext.m_hasPosition)
            {
                wmLog(eDebug, "No scanner position in context \n");
            }

        }
    }

	const interface::GeoDoublearray		oGeoDoublearrayOut(rSampleFrameIn.context(), oDoublearray, rSampleFrameIn.analysisResult() ,interface::Limit);
    preSignalAction(); m_oPipeOut.signal(oGeoDoublearrayOut);
}


} // namespace filter
} // namespace precitec

