#include "pointListSink.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

PointListSink::PointListSink()
    : SinkFilter(std::string{"PointListSink"}, Poco::UUID{"316ead65-82b3-4dff-bed9-09a694c4eebb"})
{
    setInPipeConnectors({{Poco::UUID{"621cfa21-178e-4037-9cff-f3307acc9d62"}, fliplib::PipeConnector::DataType::PointList, std::string{"Bridge"}, 0}});
}

}
}
}
