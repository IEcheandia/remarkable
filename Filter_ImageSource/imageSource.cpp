/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief
 */

#include "imageSource.h"

#include <iostream>
#include <sstream>

#include <fliplib/Exception.h>
#include <fliplib/TypeToDataTypeImpl.h>

#include <vector>
#include "event/sensor.h"
#include "module/moduleLogger.h"

#include "filter/sensorFilterInterface.h"
#include "util/calibDataSingleton.h"
#include "math/3D/projectiveMathStructures.h"

using namespace fliplib;
namespace precitec {
	using namespace image;
	using namespace interface;
	using namespace geo2d;
namespace filter {

const std::string ImageSource::m_oFilterName	= std::string("ImageSource");
const std::string ImageSource::m_oPipeName0		= std::string("ImageFrame");


ImageSource::ImageSource() :
	SourceFilter	( ImageSource::m_oFilterName, Poco::UUID("CABA9D15-30DD-4a0c-93E5-3000D68068F7"), eImageSensorDefault ),  // from \DatabaseSkripts\FilterImageSource\ImageSource.sql) ),
	m_pPipeFrameIn	( nullptr ),
	m_oPipeFrameOut	( this, ImageSource::m_oPipeName0 )
{
	parameters_.add("GridSize", Parameter::TYPE_double, 1.0);

    setOutPipeConnectors({{Poco::UUID{"EBA8F843-6403-4e02-9000-BF68C946783E"}, &m_oPipeFrameOut, m_oPipeName0, 0, ""}});
    setVariantID(Poco::UUID("52477129-33EE-4bad-A3DC-0B8AC907EBAD"));
}

bool ImageSource::subscribe(BasePipe& pipe, int group)
{
	if (pipe.name() != precitec::interface::SensorFilterInterface::SENSOR_IMAGE_FRAME_PIPE) {
		return false;
	}
	m_pPipeFrameIn	= dynamic_cast< SynchronePipe < ImageFrame> * >(&pipe);
	return BaseFilter::subscribe( pipe, group );
} // subscribe



void ImageSource::proceed(const void* sender, PipeEventArgs& e) {
	poco_assert_dbg(m_pPipeFrameIn != nullptr); // to be asserted by graph editor

	const auto oFrameIn =   m_pPipeFrameIn->read(m_oCounter);

    preSignalAction();
	m_oPipeFrameOut.signal( oFrameIn );
} // proceed


} // namespace
} // namespace



