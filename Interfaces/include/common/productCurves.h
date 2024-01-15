#pragma once

#include "InterfacesManifest.h"
#include "message/serializer.h"

#include "Poco/UUID.h"

namespace precitec
{
namespace interface
{

class INTERFACES_API ProductCurves : public system::message::Serializable
{

public:
    ProductCurves();

    void serialize (system::message::MessageBuffer& p_rBuffer) const;

    void deserialize (const system::message::MessageBuffer& p_rBuffer);

    std::vector<Poco::UUID> m_curveSets;
};

}
}


