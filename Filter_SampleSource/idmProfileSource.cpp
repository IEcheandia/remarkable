/*!
 *  @file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Stefan Birmanns (SB)
 *  @date			2017
 *	@brief			Special idm profile source filter that emits a line profile vector.
 */

#include "idmProfileSource.h"

#include "event/sensor.h"
#include "filter/sensorFilterInterface.h"
#include "geo/array.h"

#include <algorithm>					///< for_each
#include <functional>					///< bind
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace image;
	using namespace interface;

namespace filter {

const std::string IDMProfileSource::m_oFilterName 		= std::string("IDMProfileSource");
const std::string IDMProfileSource::PIPENAME_LINEOUT	= std::string("LineOut");
const std::string IDMProfileSource::PIPENAME_IMAGEOUT = std::string("ImageOut");


IDMProfileSource::IDMProfileSource() : SourceFilter( IDMProfileSource::m_oFilterName, Poco::UUID{"18db6434-e427-466b-b545-5d4c33314315"}, eIDMTrackingLine ),
	m_pSampleIn( nullptr ),
	m_oLinePipeOut( this, IDMProfileSource::PIPENAME_LINEOUT ),
    m_oImagePipeOut( this, IDMProfileSource::PIPENAME_IMAGEOUT),
	m_oLineOut(1) // 1 profile
{
    setOutPipeConnectors({{Poco::UUID("3185f4fb-7ceb-4fb8-84e9-27420cc54d33"), &m_oLinePipeOut, PIPENAME_LINEOUT, 0, ""},
    {Poco::UUID("92CBC490-7E2D-4516-919B-F2DEA45DB80F"), &m_oImagePipeOut, PIPENAME_IMAGEOUT, 0, ""}});
    setVariantID(Poco::UUID("4e31a769-948a-482c-b794-b4f80ab98094"));
}



void IDMProfileSource::setParameter()
{
	SourceFilter::setParameter();
}



bool IDMProfileSource::subscribe(BasePipe& p_rPipe, int p_oGroup)
{
	if (p_rPipe.name() == precitec::interface::SensorFilterInterface::SENSOR_SAMPLE_FRAME_PIPE)
	{
		m_pSampleIn  = dynamic_cast< fliplib::SynchronePipe<interface::SampleFrame>* >(&p_rPipe);
		return BaseFilter::subscribe( p_rPipe, p_oGroup );
	}
	return false;
}



void IDMProfileSource::proceed(const void* p_pSender, PipeEventArgs& p_rEventArgs)
{
	poco_assert_dbg(m_pSampleIn != nullptr); // to be asserted by graph editor

	const interface::SampleFrame	&rSampleFrameIn = m_pSampleIn->read(m_oCounter);
	const image::Sample				&rSample(rSampleFrameIn.data());

	// todo: what if Sample.numElements() is zero? Shall we handle that case?

	// initialize the profile with the correct length, zeros as values and bad rank
	std::for_each( m_oLineOut.begin(), m_oLineOut.end(), std::bind(
			&geo2d::Doublearray::assign,
			std::placeholders::_1,
			rSample.numElements(),
			0,
			eRankMin) );

	// ok, now lets copy the coordinates over
	for ( int i = 0; i < rSample.numElements(); i++ )
	{
		m_oLineOut[0].getData()[i]	= double(rSample[i]);
		m_oLineOut[0].getRank()[i]	= eRankMax;

	}

	// now send the profile out and and image to be used for the synchronization of the other filters...
	const interface::GeoVecDoublearray  oGeoLineOut( rSampleFrameIn.context(), m_oLineOut, rSampleFrameIn.analysisResult(), 1.0 );
    const interface::ImageFrame oFrameOut(rSampleFrameIn.context(), BImage(geo2d::Size(4, 4)), rSampleFrameIn.analysisResult());

    preSignalAction();
    m_oLinePipeOut.signal( oGeoLineOut );
    m_oImagePipeOut.signal(oFrameOut);
}


} // namespace filter
} // namespace precitec

