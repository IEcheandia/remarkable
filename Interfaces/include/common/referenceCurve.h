#pragma once

#include "InterfacesManifest.h"
#include "message/serializer.h"

#include "Poco/UUID.h"
#include "geo/point.h"

namespace precitec
{
namespace interface
{

class INTERFACES_API ReferenceCurve : public precitec::system::message::Serializable
{

public:
    ReferenceCurve(const Poco::UUID& uuid = Poco::UUID::null());

    void serialize (precitec::system::message::MessageBuffer& p_rBuffer) const;

    void deserialize (const precitec::system::message::MessageBuffer& p_rBuffer);

    Poco::UUID m_uuid;
    geo2d::VecDPoint m_curve;
};

}
}


