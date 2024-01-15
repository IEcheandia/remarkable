#ifndef ETHERCATOUTPUTSSERVER_H_
#define ETHERCATOUTPUTSSERVER_H_

#include "event/ethercatOutputs.handler.h"
#include "EtherCATMaster/EtherCATMaster.h"

namespace precitec
{

namespace ethercat
{

/**
 * EthercatOutputsServer
 **/
class EthercatOutputsServer : public TEthercatOutputs<AbstractInterface>
{
	public:

		/**
		 * Ctor.
		 * @param _service Service
		 * @return void
		 **/
		EthercatOutputsServer(EtherCATMaster& p_rEtherCATMaster);
		virtual ~EthercatOutputsServer();

		/**
		 * Kritischer Fehler -> gehe in NotReady Zustand
		 * @param relatedException Ursache fuer notReady Zustand
		 */
		virtual void ecatDigitalOut(EcatProductIndex productIndex, EcatInstance instance, uint8_t value, uint8_t mask)
		{
			m_rEtherCATMaster.ecatDigitalOut(productIndex, instance, value, mask);
		}

		virtual void ecatAnalogOut(EcatProductIndex productIndex, EcatInstance instance, EcatChannel channel, uint16_t value)
		{
			m_rEtherCATMaster.ecatAnalogOut(productIndex, instance, channel, value);
		}

		virtual void ecatGatewayOut(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, stdVecUINT8 data, stdVecUINT8 mask)
		{
			m_rEtherCATMaster.ecatGatewayOut(productIndex, instance, size, data, mask);
		}

		virtual void ecatEncoderOut(EcatProductIndex productIndex, EcatInstance instance, uint16_t command, uint32_t setCounterValue)
		{
			m_rEtherCATMaster.ecatEncoderOut(productIndex, instance, command, setCounterValue);
		}

		virtual void ecatAxisOut(EcatProductIndex productIndex, EcatInstance instance, EcatAxisOutput axisOutput)
		{
			m_rEtherCATMaster.ecatAxisOut(productIndex, instance, axisOutput);
		}

		virtual void ecatRequestSlaveInfo(void)
		{
			m_rEtherCATMaster.ecatRequestSlaveInfo();
		}

        void sendAllData(bool enable) override
        {
            m_rEtherCATMaster.sendAllData(enable);
        }

		virtual void ecatFRONTENDOut(EcatProductIndex productIndex, EcatInstance instance, EcatFRONTENDOutput frontendOutput)
		{
			m_rEtherCATMaster.ecatFRONTENDOut(productIndex, instance, frontendOutput);
		}

	private:
		EtherCATMaster &m_rEtherCATMaster;
};

} // namespace ethercat

} // namespace precitec

#endif // ETHERCATOUTPUTSSERVER_H_

