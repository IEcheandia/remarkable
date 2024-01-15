#include "lineSource.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

LineSource::LineSource()
    : SourceFilter(std::string{"LineSource"}, Poco::UUID{"5685257d-9af0-4f3d-adf4-2d17d1141312"})
{
    setOutPipeConnectors({{Poco::UUID{"c3b487e4-10e7-471e-af71-859ed6eed72a"}, fliplib::PipeConnector::DataType::Line, std::string{"Bridge"}, 0}});
}

}
}
}
