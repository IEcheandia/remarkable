/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       23.11.2016
 *  @brief      Part of the EthercatOutputs Interface
 *  @details
 */

#ifndef ETHERCATOUTPUTS_HANDLER_H_
#define ETHERCATOUTPUTS_HANDLER_H_

#include "event/ethercatOutputs.h"
#include "event/ethercatOutputs.interface.h"
#include "server/eventHandler.h"

namespace precitec
{

namespace interface
{

	using namespace  message;

	template <>
	class TEthercatOutputs<EventHandler> : public Server<EventHandler>, public TEthercatOutputsMessageDefinition
	{
	public:
		EVENT_HANDLER( TEthercatOutputs );
	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_EVENT(EcatDigitalOut, ecatDigitalOut);
			REGISTER_EVENT(EcatAnalogOut, ecatAnalogOut);
			REGISTER_EVENT(EcatGatewayOut, ecatGatewayOut);
			REGISTER_EVENT(EcatEncoderOut, ecatEncoderOut);
			REGISTER_EVENT(EcatAxisOut, ecatAxisOut);
			REGISTER_EVENT(EcatRequestSlaveInfo, ecatRequestSlaveInfo);
            REGISTER_EVENT(SendAllData, sendAllData);
			REGISTER_EVENT(EcatFRONTENDOut, ecatFRONTENDOut);
		}

		void ecatDigitalOut(Receiver &receiver)
		{
			EcatProductIndex productIndex; receiver.deMarshal(productIndex);
			EcatInstance instance; receiver.deMarshal(instance);
			uint8_t value; receiver.deMarshal(value);
			uint8_t mask; receiver.deMarshal(mask);
			getServer()->ecatDigitalOut(productIndex, instance, value, mask);
		}

		void ecatAnalogOut(Receiver &receiver)
		{
			EcatProductIndex productIndex; receiver.deMarshal(productIndex);
			EcatInstance instance; receiver.deMarshal(instance);
			EcatChannel channel; receiver.deMarshal(channel);
			uint16_t value; receiver.deMarshal(value);
			getServer()->ecatAnalogOut(productIndex, instance, channel, value);
		}

		void ecatGatewayOut(Receiver &receiver)
		{
			EcatProductIndex productIndex; receiver.deMarshal(productIndex);
			EcatInstance instance; receiver.deMarshal(instance);
			uint8_t size; receiver.deMarshal(size);

			stdVecUINT8 data;
			for(uint8_t i = 0;i < size;i++)
			{
				uint8_t value; receiver.deMarshal(value);
				data.push_back(value);
			}
			stdVecUINT8 mask;
			for(uint8_t i = 0;i < size;i++)
			{
				uint8_t value; receiver.deMarshal(value);
				mask.push_back(value);
			}
			getServer()->ecatGatewayOut(productIndex, instance, size, data, mask);
		}

		void ecatEncoderOut(Receiver &receiver)
		{
			EcatProductIndex productIndex; receiver.deMarshal(productIndex);
			EcatInstance instance; receiver.deMarshal(instance);
			uint16_t command; receiver.deMarshal(command);
			uint32_t setCounterValue; receiver.deMarshal(setCounterValue);
			getServer()->ecatEncoderOut(productIndex, instance, command, setCounterValue);
		}

		void ecatAxisOut(Receiver &receiver)
		{
			EcatProductIndex productIndex; receiver.deMarshal(productIndex);
			EcatInstance instance; receiver.deMarshal(instance);
			EcatAxisOutput axisOutput; receiver.deMarshal(axisOutput);
			getServer()->ecatAxisOut(productIndex, instance, axisOutput);
		}

		void ecatRequestSlaveInfo(Receiver &receiver)
		{
			getServer()->ecatRequestSlaveInfo();
		}

        void sendAllData(Receiver &receiver)
        {
            bool enable;
            receiver.deMarshal(enable);
            getServer()->sendAllData(enable);
        }

		void ecatFRONTENDOut(Receiver &receiver)
		{
			EcatProductIndex productIndex; receiver.deMarshal(productIndex);
			EcatInstance instance; receiver.deMarshal(instance);
			EcatFRONTENDOutput frontendOutput; receiver.deMarshal(frontendOutput);
			getServer()->ecatFRONTENDOut(productIndex, instance, frontendOutput);
		}

	private:
		TEthercatOutputs<AbstractInterface> * getServer()
		{
			return server_;
		}

	};

} // namespace interface
} // namespace precitec

#endif /* ETHERCATOUTPUTS_HANDLER_H_ */

