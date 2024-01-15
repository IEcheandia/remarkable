#include "poorPenetrationCandidateSink.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

PoorPenetrationCandidateSink::PoorPenetrationCandidateSink()
    : SinkFilter(std::string{"PoorPenetrationCandidateSink"}, Poco::UUID{"523ce7df-d6a8-44da-8263-a0d500307e3c"})
{
    setInPipeConnectors({{Poco::UUID{"5a35a1b5-f395-4b3e-90c1-cda4855a7375"}, fliplib::PipeConnector::DataType::PoorPenetrationCandidate, std::string{"Bridge"}, 0}});
}

}
}
}
