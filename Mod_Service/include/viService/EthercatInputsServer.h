#ifndef ETHERCATINPUTSSERVER_H_
#define ETHERCATINPUTSSERVER_H_

#include "event/ethercatInputsToService.handler.h"
#include "serviceToGuiServer.h"
#include "serviceFromGuiServer.h"

namespace precitec
{

using namespace ethercat;

namespace viService
{

/**
 * EthercatInputsServer
 **/
class EthercatInputsServer : public TEthercatInputsToService<AbstractInterface>
{
	public:

		EthercatInputsServer(ServiceToGuiServer& p_rServiceToGuiServer, ServiceFromGuiServer& p_rServiceFromGuiServer);
		~EthercatInputsServer() override;


		void ecatAllDataIn(uint16_t size, stdVecUINT8 data) override
		{
			m_rServiceToGuiServer.ecatAllDataIn(size, data);
		}

		void ecatAllSlaveInfo(SlaveInfo p_oSlaveInfo) override
		{
            m_rServiceToGuiServer.SlaveInfoECAT(p_oSlaveInfo.GetSize(), p_oSlaveInfo);

            std::string foo;
            m_rServiceToGuiServer.ConfigInfo(foo);

            m_rServiceFromGuiServer.Init(p_oSlaveInfo);
        }

		void fieldbusAllDataIn(uint16_t size, stdVecUINT8 data) override
		{
			m_rServiceToGuiServer.fieldbusAllDataIn(size, data);
		}

		void fieldbusAllSlaveInfo(SlaveInfo p_oSlaveInfo) override
		{
            m_rServiceToGuiServer.SlaveInfoFieldbus(p_oSlaveInfo.GetSize(), p_oSlaveInfo);

            std::string foo;
            m_rServiceToGuiServer.ConfigInfo(foo);

            m_rServiceFromGuiServer.Init(p_oSlaveInfo);
        }

        void NewSlaveInfo(SlaveInfo p_oSlaveInfo); // Callback from serviceToGuiServer

	private:
		ServiceToGuiServer &m_rServiceToGuiServer;
		ServiceFromGuiServer &m_rServiceFromGuiServer;
};

} // namespace viService

} // namespace precitec

#endif // ETHERCATINPUTSSERVER_H_

