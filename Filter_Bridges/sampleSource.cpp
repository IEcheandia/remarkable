#include "sampleSource.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

SampleSource::SampleSource()
    : SourceFilter(std::string{"SampleSource"}, Poco::UUID{"21950067-eb43-4a7b-9707-5080b2153733"})
{
    setOutPipeConnectors({{Poco::UUID{"b63cf883-9b5c-4fc1-891d-0580728061ee"}, fliplib::PipeConnector::DataType::Sample, std::string{"Bridge"}, 0}});
}

}
}
}
