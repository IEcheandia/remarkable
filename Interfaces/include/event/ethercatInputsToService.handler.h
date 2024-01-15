#pragma once

#include "event/ethercatInputs.h"
#include "event/ethercatInputsToService.interface.h"
#include "server/eventHandler.h"

namespace precitec
{

namespace interface
{

using namespace  message;

template <>
class TEthercatInputsToService<EventHandler> : public Server<EventHandler>, public TEthercatInputsToServiceMessageDefinition
{
public:
    EVENT_HANDLER( TEthercatInputsToService );
public:
    void registerCallbacks()
    {
        REGISTER_EVENT(EcatAllDataIn, ecatAllDataIn);
        REGISTER_EVENT(EcatAllSlaveInfo, ecatAllSlaveInfo);
        REGISTER_EVENT(FieldbusAllDataIn, fieldbusAllDataIn);
        REGISTER_EVENT(FieldbusAllSlaveInfo, fieldbusAllSlaveInfo);
    }

    void ecatAllDataIn(Receiver &receiver)
    {
        uint16_t size; receiver.deMarshal(size);

        stdVecUINT8 data;
        for(uint16_t i = 0;i < size;i++)
        {
            uint8_t value; receiver.deMarshal(value);
            data.push_back(value);
        }
        getServer()->ecatAllDataIn(size, data);
    }

    void ecatAllSlaveInfo(Receiver &receiver)
    {
        // oSlaveInfo(1): 1 is a dummy for the Ctor
        SlaveInfo oSlaveInfo(1); receiver.deMarshal(oSlaveInfo);
        getServer()->ecatAllSlaveInfo(oSlaveInfo);
    }

    void fieldbusAllDataIn(Receiver &receiver)
    {
        uint16_t size; receiver.deMarshal(size);

        stdVecUINT8 data;
        for(uint16_t i = 0;i < size;i++)
        {
            uint8_t value; receiver.deMarshal(value);
            data.push_back(value);
        }
        getServer()->fieldbusAllDataIn(size, data);
    }

    void fieldbusAllSlaveInfo(Receiver &receiver)
    {
        // oSlaveInfo(1): 1 is a dummy for the Ctor
        SlaveInfo oSlaveInfo(1); receiver.deMarshal(oSlaveInfo);
        getServer()->fieldbusAllSlaveInfo(oSlaveInfo);
    }

private:
    TEthercatInputsToService<AbstractInterface> * getServer()
    {
        return server_;
    }

};

} // namespace interface
} // namespace precitec
