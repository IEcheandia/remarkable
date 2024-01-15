#include "sampleSink.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

SampleSink::SampleSink()
    : SinkFilter(std::string{"SampleSink"}, Poco::UUID{"f1a572c8-8e30-4792-83bc-7508d21a533d"})
{
    setInPipeConnectors({{Poco::UUID{"b0462216-dff3-4ba8-833c-5a9418357c7d"}, fliplib::PipeConnector::DataType::Sample, std::string{"Bridge"}, 0}});
}

}
}
}
