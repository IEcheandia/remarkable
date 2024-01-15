#include "seamFindingSink.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

SeamFindingSink::SeamFindingSink()
    : SinkFilter(std::string{"SeamFindingSink"}, Poco::UUID{"17b929b2-30ec-4c67-b0e3-4f319d05e50b"})
{
    setInPipeConnectors({{Poco::UUID{"85a109f0-cd29-4c62-90c4-04efacf437ae"}, fliplib::PipeConnector::DataType::SeamFinding, std::string{"Bridge"}, 0}});
}

}
}
}
