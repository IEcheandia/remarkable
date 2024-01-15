#include "seamFindingSource.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

SeamFindingSource::SeamFindingSource()
    : SourceFilter(std::string{"SeamFindingSource"}, Poco::UUID{"18ece4c7-b1f5-4cf5-8b9d-4de7971a1d6d"})
{
    setOutPipeConnectors({{Poco::UUID{"f26616d6-3378-44dc-ab95-11dcc8922cb4"}, fliplib::PipeConnector::DataType::SeamFinding, std::string{"Bridge"}, 0}});
}

}
}
}
