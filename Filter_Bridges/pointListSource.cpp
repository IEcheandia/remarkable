#include "pointListSource.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

PointListSource::PointListSource()
    : SourceFilter(std::string{"PointListSource"}, Poco::UUID{"55971812-0b2e-4202-bfbd-e7821a9044f6"})
{
    setOutPipeConnectors({{Poco::UUID{"3591e609-4e6a-4b3f-8f08-9002c23a1a05"}, fliplib::PipeConnector::DataType::PointList, std::string{"Bridge"}, 0}});
}

}
}
}
