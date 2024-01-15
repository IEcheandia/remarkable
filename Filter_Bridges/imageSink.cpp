#include "imageSink.h"

namespace precitec
{
namespace filter
{
namespace bridges
{

ImageSink::ImageSink()
    : SinkFilter(std::string{"ImageSink"}, Poco::UUID{"3f59b49e-7fa3-4fa8-9317-e21947b26bd8"})
{
    setInPipeConnectors({{Poco::UUID{"3aa04b72-f0b9-470b-8f40-af2b07b4c78f"}, fliplib::PipeConnector::DataType::Image, std::string{"Bridge"}, 0}});
}

}
}
}
