#include "sampleToImageSource.h"

#include "event/sensor.h"
#include "filter/sensorFilterInterface.h"

#include <fliplib/Exception.h>
#include <fliplib/TypeToDataTypeImpl.h>

#include "module/moduleLogger.h"

namespace precitec {
namespace filter {

SampleToImageSource::SampleToImageSource()
    : SourceFilter("SampleToImageSource", Poco::UUID("7f678e65-8db6-471a-8bc3-c132f15b4357"), interface::eExternSensorDefault)
    , m_sampleIn(nullptr)
    , m_pipeFrameOut(this, "ImageFrame")
{
    setOutPipeConnectors({{Poco::UUID{"04b69ab4-5dfa-496e-b794-c9692f9ef4cd"}, &m_pipeFrameOut, "ImageFrame", 0, ""}});
    setVariantID(Poco::UUID("a724729d-da03-465d-b575-386af2a623b0"));
}

bool SampleToImageSource::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.name() == precitec::interface::SensorFilterInterface::SENSOR_SAMPLE_FRAME_PIPE)
    {
        m_sampleIn  = dynamic_cast<fliplib::SynchronePipe<interface::SampleFrame>*>(&pipe);
        return BaseFilter::subscribe(pipe, group);
    }
    return false;
}

void SampleToImageSource::proceed(const void* sender, fliplib::PipeEventArgs& e) {
    const interface::SampleFrame &sampleFrameIn = m_sampleIn->read(m_oCounter);

    image::BImage image;
    interface::ImageFrame imageFrame(sampleFrameIn.context(), image);

    preSignalAction();
    m_pipeFrameOut.signal(imageFrame);
}

} // namespace filter
} // namespace precitec
