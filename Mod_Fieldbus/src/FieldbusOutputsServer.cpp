#include "Fieldbus/FieldbusOutputsServer.h"

namespace precitec
{

namespace ethercat
{

FieldbusOutputsServer::FieldbusOutputsServer(Fieldbus& p_rFieldbus)
        :m_rFieldbus(p_rFieldbus)
{
}

FieldbusOutputsServer::~FieldbusOutputsServer()
{
}

} // namespace ethercat

} // namespace precitec;

