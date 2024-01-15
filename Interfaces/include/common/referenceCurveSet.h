#pragma once

#include "InterfacesManifest.h"
#include "message/serializer.h"

#include "Poco/UUID.h"
#include "common/referenceCurve.h"

namespace precitec
{
namespace interface
{

class INTERFACES_API ReferenceCurveSet : public precitec::system::message::Serializable
{

public:
    ReferenceCurveSet(const Poco::UUID& uuid = Poco::UUID::null());
    ReferenceCurveSet(const int32_t& type, const uint32_t& seam, const uint32_t& seamSeries, const Poco::UUID& uuid = Poco::UUID::null());

    void serialize (system::message::MessageBuffer& p_rBuffer) const;

    void deserialize (const system::message::MessageBuffer& p_rBuffer);

    Poco::UUID m_uuid;
    int32_t m_type;
    uint32_t m_seam;
    uint32_t m_seamSeries;

    ReferenceCurve m_upper;
    ReferenceCurve m_middle;
    ReferenceCurve m_lower;
};

}
}

