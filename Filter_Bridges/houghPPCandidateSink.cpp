#include "houghPPCandidateSink.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

HoughPPCandidateSink::HoughPPCandidateSink()
    : SinkFilter(std::string{"HoughPPCandidateSink"}, Poco::UUID{"eff5a773-c29c-42c7-b804-dcafe5414dee"})
{
    setInPipeConnectors({{Poco::UUID{"d0b8486e-78ea-4732-944c-72c2f3aec489"}, fliplib::PipeConnector::DataType::HoughPPCandidate, std::string{"Bridge"}, 0}});
}

}
}
}
