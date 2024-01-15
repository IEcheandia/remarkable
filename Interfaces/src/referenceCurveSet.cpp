#include "common/referenceCurveSet.h"

using precitec::system::message::MessageBuffer;

namespace precitec
{
namespace interface
{

ReferenceCurveSet::ReferenceCurveSet(const Poco::UUID& uuid)
    : m_uuid(uuid)
    , m_type {0}
    , m_seam {0}
    , m_seamSeries {0}
{
}

ReferenceCurveSet::ReferenceCurveSet(const int32_t& type, const uint32_t& seam, const uint32_t& seamSeries, const Poco::UUID& uuid)
    : m_uuid(uuid)
    , m_type {type}
    , m_seam {seam}
    , m_seamSeries {seamSeries}
{
}

void ReferenceCurveSet::serialize(MessageBuffer& p_rBuffer) const
{
    marshal(p_rBuffer, m_uuid);
    marshal(p_rBuffer, m_type);
    marshal(p_rBuffer, m_seam);
    marshal(p_rBuffer, m_seamSeries);
    marshal(p_rBuffer, m_upper);
    marshal(p_rBuffer, m_middle);
    marshal(p_rBuffer, m_lower);
}

void ReferenceCurveSet::deserialize(const MessageBuffer& p_rBuffer)
{
    deMarshal(p_rBuffer, m_uuid);
    deMarshal(p_rBuffer, m_type);
    deMarshal(p_rBuffer, m_seam);
    deMarshal(p_rBuffer, m_seamSeries);
    deMarshal(p_rBuffer, m_upper);
    deMarshal(p_rBuffer, m_middle);
    deMarshal(p_rBuffer, m_lower);
}

}
}
