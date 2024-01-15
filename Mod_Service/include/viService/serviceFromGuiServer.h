#ifndef SERVICEFROMGUISERVER_H_
#define SERVICEFROMGUISERVER_H_

#include "serviceToGuiServer.h"
#include "event/viService.h"
#include "event/viServiceFromGUI.interface.h"
#include "event/viServiceFromGUI.proxy.h"

#include "event/ethercatOutputs.proxy.h"

namespace precitec
{
    using namespace interface;
    using namespace viService;

namespace ethercat
{
    class Service;

    /**
    * Send help struct
    **/
    struct SENDER_INFORMATION
    {
        EC_T_GET_SLAVE_INFO info;
        int instanceID;
        short type;
        EcatInstance m_oEcatInstance;
        EcatProductIndex m_oEcatProductIndex;
        EcatChannel m_oEcatChannel;
    };

    /**
    * Service-Schnittstelle (wird vom Win-Host aufgerufen)
    **/
    class ServiceFromGuiServer : public TviServiceFromGUI<AbstractInterface>
    {

    public:
        /**
        * Ctor.
        * @param outServiceDataServer
        * @param cbSendDataToProxy
        * @return
        */
        ServiceFromGuiServer(ServiceToGuiServer &serviceToGuiServer, TEthercatOutputs<EventProxy>& p_rEthercatOutputsProxy);
        virtual ~ServiceFromGuiServer();

        /**
        *
        * @param slaveNr
        * @param data
        * @param mask
        */
        virtual void OutputProcessData(short physAddr, ProcessData& data, ProcessData& mask, short type);

        /**
        * Aktiviert bzw. deaktiviert die Datenuebertragung der Feldbusdaten (nur von der GUI aus)
        * @param onOff
        */
        virtual void SetTransferMode(bool onOff);

        void requestSlaveInfo() override;

        void Init(SlaveInfo slaveInfo);

        void OutgoingData(short physAddr, short size, char* data, char* mask);

    private:
        ServiceToGuiServer &m_serviceToGuiServer;
        TEthercatOutputs<EventProxy>& m_rEthercatOutputsProxy;
        std::vector<SENDER_INFORMATION > m_senderList;

        void AddSenderInfo(EC_T_GET_SLAVE_INFO info, int instanceID, short type = 1);
    };

} // namespace ethercat

} // namespace precitec

#endif /* SERVICEFROMGUISERVER_H_ */

