#pragma once

#include "event/S6K_InfoFromProcesses.interface.h"
#include "server/eventHandler.h"

namespace precitec
{

namespace interface
{

using namespace  message;

template <>
class TS6K_InfoFromProcesses<EventHandler> : public Server<EventHandler>, public TS6K_InfoFromProcessesMessageDefinition
{
public:
    EVENT_HANDLER( TS6K_InfoFromProcesses );
public:
    void registerCallbacks()
    {
        REGISTER_EVENT(MaxSouvisSpeed, maxSouvisSpeed);
        REGISTER_EVENT(SouvisControlBits, souvisControlBits);
        REGISTER_EVENT(PassS6K_CS_DataBlock_To_Inspect, passS6K_CS_DataBlock_To_Inspect);
        REGISTER_EVENT(NumberOfPresentSeams, numberOfPresentSeams);
        REGISTER_EVENT(ProductNoFromTCP, productNoFromTCP);
    }

    void maxSouvisSpeed(Receiver &receiver)
    {
        uint32_t oSpeed; receiver.deMarshal(oSpeed);
        getServer()->maxSouvisSpeed(oSpeed);
    }

    void souvisControlBits(Receiver &receiver)
    {
        bool oSouvisPresent; receiver.deMarshal(oSouvisPresent);
        bool oSouvisSelected; receiver.deMarshal(oSouvisSelected);
        getServer()->souvisControlBits(oSouvisPresent, oSouvisSelected);
    }

    void passS6K_CS_DataBlock_To_Inspect(Receiver &receiver)
    {
        uint32_t oProductNo; receiver.deMarshal(oProductNo);
        uint32_t oBatchID; receiver.deMarshal(oBatchID);
        uint16_t oSeamNo; receiver.deMarshal(oSeamNo);
        uint16_t oBlockNo; receiver.deMarshal(oBlockNo);
        uint16_t oFirstMeasureInBlock; receiver.deMarshal(oFirstMeasureInBlock);
        uint16_t oMeasureCntInBlock; receiver.deMarshal(oMeasureCntInBlock);
        uint16_t oMeasuresPerResult; receiver.deMarshal(oMeasuresPerResult);
        uint16_t oValuesPerMeasure; receiver.deMarshal(oValuesPerMeasure);
        CS_BlockType oCS_DataBlock; receiver.deMarshal(oCS_DataBlock);
        getServer()->passS6K_CS_DataBlock_To_Inspect(oProductNo, oBatchID, oSeamNo, oBlockNo, oFirstMeasureInBlock, oMeasureCntInBlock, oMeasuresPerResult, oValuesPerMeasure, oCS_DataBlock);
    }

    void numberOfPresentSeams(Receiver &receiver)
    {
        uint32_t oSeams; receiver.deMarshal(oSeams);
        getServer()->numberOfPresentSeams(oSeams);
    }

    void productNoFromTCP(Receiver &receiver)
    {
        uint32_t oProductNo; receiver.deMarshal(oProductNo);
        getServer()->productNoFromTCP(oProductNo);
    }

private:
    TS6K_InfoFromProcesses<AbstractInterface> * getServer()
    {
        return server_;
    }

};

} // namespace interface
} // namespace precitec
