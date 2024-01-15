#include "blobSink.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

BlobsSink::BlobsSink()
    : SinkFilter(std::string{"BlobsSink"}, Poco::UUID{"b50e317b-9895-4d0e-a547-8d8ce9cdaf3f"})
{
    setInPipeConnectors({{Poco::UUID{"4f938b9e-45b1-482b-8891-a537525540ed"}, fliplib::PipeConnector::DataType::Blobs, std::string{"Bridge"}, 0}});
}

}
}
}
