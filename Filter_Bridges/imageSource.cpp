#include "imageSource.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

ImageSource::ImageSource()
    : SourceFilter(std::string{"ImageSource"}, Poco::UUID{"8310c333-20a2-4a91-a9f9-e72199d0604b"})
{
    setOutPipeConnectors({{Poco::UUID{"0efa222b-906c-41ba-86e6-49c8fb06b8ef"}, fliplib::PipeConnector::DataType::Image, std::string{"Bridge"}, 0}});
}

}
}
}
