#include "common/productCurves.h"

using precitec::system::message::MessageBuffer;

namespace precitec
{
namespace interface
{

ProductCurves::ProductCurves()
{
}

void ProductCurves::serialize(MessageBuffer& p_rBuffer) const
{
    marshal(p_rBuffer, m_curveSets);
}

void ProductCurves::deserialize(const MessageBuffer& p_rBuffer)
{
    deMarshal(p_rBuffer, m_curveSets);
}

}
}



