#include "doubleSink.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

DoubleSink::DoubleSink()
    : SinkFilter(std::string{"DoubleSink"}, Poco::UUID{"488967c8-02f2-4a18-b88d-7f2c5b9749b4"})
{
    setInPipeConnectors({{Poco::UUID{"4c9eab1e-685a-4a8e-a811-e91f81fb04a0"}, fliplib::PipeConnector::DataType::Double, std::string{"Bridge"}, 0}});
}

}
}
}
