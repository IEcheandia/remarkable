/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       23.11.2016
 *  @brief      Part of the EthercatOutputs Interface
 *  @details
 */

#ifndef ETHERCATOUTPUTS_PROXY_H_
#define ETHERCATOUTPUTS_PROXY_H_

#include "event/ethercatOutputs.h"
#include "event/ethercatOutputs.interface.h"
#include "server/eventProxy.h"

namespace precitec
{

namespace interface
{

	template <>
	class TEthercatOutputs<EventProxy> : public Server<EventProxy>, public TEthercatOutputs<AbstractInterface>, public TEthercatOutputsMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TEthercatOutputs() : EVENT_PROXY_CTOR(TEthercatOutputs), TEthercatOutputs<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

		virtual ~TEthercatOutputs() {}

	public:

		virtual void ecatDigitalOut(EcatProductIndex productIndex, EcatInstance instance, uint8_t value, uint8_t mask)
		{
			INIT_EVENT(EcatDigitalOut);
			signaler().marshal(productIndex);
			signaler().marshal(instance);
			signaler().marshal(value);
			signaler().marshal(mask);
			signaler().send();
		}

		virtual void ecatAnalogOut(EcatProductIndex productIndex, EcatInstance instance, EcatChannel channel, uint16_t value)
		{
			INIT_EVENT(EcatAnalogOut);
			signaler().marshal(productIndex);
			signaler().marshal(instance);
			signaler().marshal(channel);
			signaler().marshal(value);
			signaler().send();
		}

		virtual void ecatGatewayOut(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, stdVecUINT8 data, stdVecUINT8 mask)
		{
			INIT_EVENT(EcatGatewayOut);
			signaler().marshal(productIndex);
			signaler().marshal(instance);
			signaler().marshal(size);
			for(uint8_t i = 0;i < size;i++)
			{
				signaler().marshal(data[i]);
			}
			for(uint8_t i = 0;i < size;i++)
			{
				signaler().marshal(mask[i]);
			}
			signaler().send();
		}

		virtual void ecatEncoderOut(EcatProductIndex productIndex, EcatInstance instance, uint16_t command, uint32_t setCounterValue)
		{
			INIT_EVENT(EcatEncoderOut);
			signaler().marshal(productIndex);
			signaler().marshal(instance);
			signaler().marshal(command);
			signaler().marshal(setCounterValue);
			signaler().send();
		}

		virtual void ecatAxisOut(EcatProductIndex productIndex, EcatInstance instance, EcatAxisOutput axisOutput)
		{
			INIT_EVENT(EcatAxisOut);
			signaler().marshal(productIndex);
			signaler().marshal(instance);
			signaler().marshal(axisOutput);
			signaler().send();
		}

		virtual void ecatRequestSlaveInfo(void)
		{
			INIT_EVENT(EcatRequestSlaveInfo);
			signaler().send();
		}

        void sendAllData(bool enable) override
        {
            INIT_EVENT(SendAllData);
            signaler().marshal(enable);
            signaler().send();
        }

		virtual void ecatFRONTENDOut(EcatProductIndex productIndex, EcatInstance instance, EcatFRONTENDOutput frontendOutput)
		{
			INIT_EVENT(EcatFRONTENDOut);
			signaler().marshal(productIndex);
			signaler().marshal(instance);
			signaler().marshal(frontendOutput);
			signaler().send();
		}

	};

} // namespace interface
} // namespace precitec

#endif /* ETHERCATOUTPUTS_PROXY_H_ */

