#include "doubleSource.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

DoubleSource::DoubleSource()
    : SourceFilter(std::string{"DoubleSource"}, Poco::UUID{"7770b9a6-ca04-46a7-b27c-33df27344ad4"})
{
    setOutPipeConnectors({{Poco::UUID{"57a02792-28a5-44f7-8458-ae128abcfb10"}, fliplib::PipeConnector::DataType::Double, std::string{"Bridge"}, 0}});
}

}
}
}
