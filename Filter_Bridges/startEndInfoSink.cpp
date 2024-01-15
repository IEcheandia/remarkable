#include "startEndInfoSink.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

StartEndInfoSink::StartEndInfoSink()
    : SinkFilter(std::string{"StartEndInfoSink"}, Poco::UUID{"9d9a9a12-e51f-4921-a802-b0c90f516310"})
{
    setInPipeConnectors({{Poco::UUID{"16206c41-68cf-4141-a9e5-b128e00a964f"}, fliplib::PipeConnector::DataType::StartEndInfo, std::string{"Bridge"}, 0}});
}

}
}
}
