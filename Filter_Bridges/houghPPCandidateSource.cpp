#include "houghPPCandidateSource.h"


namespace precitec
{
namespace filter
{
namespace bridges
{

HoughPPCandidateSource::HoughPPCandidateSource()
    : SourceFilter(std::string{"HoughPPCandidateSource"}, Poco::UUID{"622b871c-c1ce-466c-89cc-376927571926"})
{
    setOutPipeConnectors({{Poco::UUID{"ae205c91-1f2c-4b43-8d2f-ac49b53e4f79"}, fliplib::PipeConnector::DataType::HoughPPCandidate, std::string{"Bridge"}, 0}});
}

}
}
}
