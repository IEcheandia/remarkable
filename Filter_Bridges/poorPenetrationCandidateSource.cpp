#include "poorPenetrationCandidateSource.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

PoorPenetrationCandidateSource::PoorPenetrationCandidateSource()
    : SourceFilter(std::string{"PoorPenetrationCandidateSource"}, Poco::UUID{"c260999e-c556-4c58-917d-f6dde7b79fc7"})
{
    setOutPipeConnectors({{Poco::UUID{"438703df-0c3e-4dc8-b539-172d248c062e"}, fliplib::PipeConnector::DataType::PoorPenetrationCandidate, std::string{"Bridge"}, 0}});
}

}
}
}
