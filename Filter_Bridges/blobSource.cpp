#include "blobSource.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

BlobsSource::BlobsSource()
    : SourceFilter(std::string{"BlobsSource"}, Poco::UUID{"59674f26-488c-4ab5-a160-8debf4928f40"})
{
    setOutPipeConnectors({{Poco::UUID{"eae9f51f-250c-4b82-9087-75ce18b554b6"}, fliplib::PipeConnector::DataType::Blobs, std::string{"Bridge"}, 0}});
}

}
}
}
