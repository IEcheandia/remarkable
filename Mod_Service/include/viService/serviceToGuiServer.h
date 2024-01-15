#ifndef SERVICETOGUISERVER_H_
#define SERVICETOGUISERVER_H_

#include "module/moduleLogger.h"
#include "event/viService.h"
#include "event/viServiceToGUI.interface.h"
#include "event/viServiceToGUI.proxy.h"
#include "event/ethercatInputs.handler.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <functional>

namespace precitec
{
    using namespace interface;

namespace viService
{

#define MAX_ETHERCAT_INPUT_BYTES     10000
#define MAX_ETHERCAT_OUTPUT_BYTES    10000
#define MAX_FIELDBUS_INPUT_BYTES     256
#define MAX_FIELDBUS_OUTPUT_BYTES    256

typedef std::function<void(SlaveInfo)> ServiceCallbackFunction;

    /**
    * Service-Schnittstelle, sendet Daten an Win-Host
    **/
    class ServiceToGuiServer : public TviServiceToGUI<AbstractInterface>
    {

public:
        /**
        * Ctor.
        * @param outServiceDataProxy
        * @return
        **/
        ServiceToGuiServer( TviServiceToGUI<EventProxy>& viServiceToGuiProxy );
        virtual ~ServiceToGuiServer();

        /**
        *
        * @param input
        * @param output
        */
        virtual void ProcessImage(ProcessDataVector& input, ProcessDataVector& output);

        /**
        * SlaveInfoECAT
        * @param count
        * @param info
        **/
        virtual void SlaveInfoECAT(short count, SlaveInfo info);

        /**
        * SlaveInfoFieldbus
        * @param count
        * @param info
        **/
        virtual void SlaveInfoFieldbus(short count, SlaveInfo info);

        virtual void ConfigInfo(std::string config);

        void SetTransferModus(bool onOff);

        void ecatAllDataIn(uint16_t size, stdVecUINT8 data);

        void fieldbusAllDataIn(uint16_t size, stdVecUINT8 data);

        void sendSlaveInfo();

        void connectCallback(ServiceCallbackFunction p_pServiceCallbackFunction);

private:
        ServiceCallbackFunction m_pServiceCallbackFunction;

        void BuildAndSendCollectedSlaveInfo(void);
        void BuildAndSendFieldbusOnlySlaveInfo(void);

        bool m_oFieldbusViaSeparateFieldbusBoard;
        bool m_etherCATMasterIsActive{true};

        ProcessDataVector m_pdInVector;
        ProcessDataVector m_pdOutVector;

        int m_oFieldbusInputSize;
        char m_oFieldbusInputBuffer[MAX_FIELDBUS_INPUT_BYTES];
        int m_oFieldbusOutputSize;
        char m_oFieldbusOutputBuffer[MAX_FIELDBUS_OUTPUT_BYTES];

        int m_countSlavesECAT;
        std::vector <EC_T_GET_SLAVE_INFO> m_slaveInfoECAT;
        int m_countSlavesFieldbus;
        std::vector <EC_T_GET_SLAVE_INFO> m_slaveInfoFieldbus;

        TviServiceToGUI<EventProxy>& viServiceToGuiProxy_;

        bool m_sendDataToHost; ///Ist die Uebertragung der Feldbusdaten an den Host fuer die Visualisierung aktiviert ?

        void IncomingDataECAT(short inSize, const char* inData, short outSize, const char* outData);
        void IncomingDataFieldbus(short inSize, const char* inData, short outSize, const char* outData);

        bool m_oSlaveInfoFromECATValid;
        bool m_oSlaveInfoFromFieldbusValid;
    };
    
} // namespace viService
} // namespace precitec

#endif /* SERVICETOGUISERVER_H_ */

