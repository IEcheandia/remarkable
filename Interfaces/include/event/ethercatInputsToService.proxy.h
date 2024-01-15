#pragma once

#include "event/ethercatInputs.h"
#include "event/ethercatInputsToService.interface.h"
#include "server/eventProxy.h"

namespace precitec
{

namespace interface
{

template <>
class TEthercatInputsToService<EventProxy> : public Server<EventProxy>, public TEthercatInputsToService<AbstractInterface>, public TEthercatInputsToServiceMessageDefinition
{
public:
    TEthercatInputsToService() : EVENT_PROXY_CTOR(TEthercatInputsToService), TEthercatInputsToService<AbstractInterface>()
    {
    }

    ~TEthercatInputsToService() override {}

    void ecatAllDataIn(uint16_t size, stdVecUINT8 data) override
    {
        INIT_EVENT(EcatAllDataIn);
        signaler().marshal(size);
        for(uint16_t i = 0;i < size;i++)
        {
            signaler().marshal(data[i]);
        }
        signaler().send();
    }

    void ecatAllSlaveInfo(SlaveInfo p_oSlaveInfo) override
    {
        INIT_EVENT(EcatAllSlaveInfo);
        signaler().marshal(p_oSlaveInfo);
        signaler().send();
    }

    void fieldbusAllDataIn(uint16_t size, stdVecUINT8 data) override
    {
        INIT_EVENT(FieldbusAllDataIn);
        signaler().marshal(size);
        for(uint16_t i = 0;i < size;i++)
        {
            signaler().marshal(data[i]);
        }
        signaler().send();
    }

    void fieldbusAllSlaveInfo(SlaveInfo p_oSlaveInfo) override
    {
        INIT_EVENT(FieldbusAllSlaveInfo);
        signaler().marshal(p_oSlaveInfo);
        signaler().send();
    }

};

} // namespace interface
} // namespace precitec
