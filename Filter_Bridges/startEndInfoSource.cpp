#include "startEndInfoSource.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

StartEndInfoSource::StartEndInfoSource()
    : SourceFilter(std::string{"StartEndInfoSource"}, Poco::UUID{"7c4c7bfa-39f1-498b-89bb-f3802b9f2f2a"})
{
    setOutPipeConnectors({{Poco::UUID{"82f5d475-79ab-4dfe-ac58-f93bcc539d28"}, fliplib::PipeConnector::DataType::StartEndInfo, std::string{"Bridge"}, 0}});
}

}
}
}
