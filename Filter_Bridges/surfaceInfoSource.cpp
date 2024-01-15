#include "surfaceInfoSource.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

SurfaceInfoSource::SurfaceInfoSource()
    : SourceFilter(std::string{"SurfaceInfoSource"}, Poco::UUID{"a8f73f37-b83d-4d6c-9cfc-4e38a53e8b7b"})
{
    setOutPipeConnectors({{Poco::UUID{"e3923ee6-d8d4-4a64-9849-3487cc325a33"}, fliplib::PipeConnector::DataType::SurfaceInfo, std::string{"Bridge"}, 0}});
}

}
}
}
