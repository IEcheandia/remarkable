#include "lineSink.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

LineSink::LineSink()
    : SinkFilter(std::string{"LineSink"}, Poco::UUID{"e108a053-874e-4942-b415-c2004a654282"})
{
    setInPipeConnectors({{Poco::UUID{"d2e612ae-1395-4b12-ae35-0697eac57cc2"}, fliplib::PipeConnector::DataType::Line, std::string{"Bridge"}, 0}});
}

}
}
}
