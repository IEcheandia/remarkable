#include "common/referenceCurve.h"

namespace precitec
{
namespace interface
{

using precitec::system::message::MessageBuffer;

ReferenceCurve::ReferenceCurve(const Poco::UUID& uuid)
    : m_uuid (uuid)
{
}

void ReferenceCurve::serialize (MessageBuffer& p_rBuffer) const {
    marshal(p_rBuffer, m_uuid);
    marshal(p_rBuffer, m_curve);
}

void ReferenceCurve::deserialize (const MessageBuffer& p_rBuffer) {
    deMarshal(p_rBuffer, m_uuid);
    deMarshal(p_rBuffer, m_curve);
}

}
}



