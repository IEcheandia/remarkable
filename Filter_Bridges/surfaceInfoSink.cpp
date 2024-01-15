#include "surfaceInfoSink.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

SurfaceInfoSink::SurfaceInfoSink()
    : SinkFilter(std::string{"SurfaceInfoSink"}, Poco::UUID{"c11907fa-2f1e-4d43-b4ba-0a88fed95d17"})
{
    setInPipeConnectors({{Poco::UUID{"7CB79008-32AC-4C6E-AFD5-A86307A569F4"}, fliplib::PipeConnector::DataType::SurfaceInfo, std::string{"Bridge"}, 0}});
}

}
}
}
