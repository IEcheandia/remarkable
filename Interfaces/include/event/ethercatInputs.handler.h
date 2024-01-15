/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       19.11.2016
 *  @brief      Part of the EthercatInputs Interface
 *  @details
 */

#ifndef ETHERCATINPUTS_HANDLER_H_
#define ETHERCATINPUTS_HANDLER_H_

#include "event/ethercatInputs.h"
#include "event/ethercatInputs.server.h"
#include "server/eventHandler.h"

namespace precitec
{

namespace interface
{

	using namespace  message;

	template <>
	class TEthercatInputs<EventHandler> : public Server<EventHandler>, public TEthercatInputsMessageDefinition
	{
	public:
		EVENT_HANDLER2( TEthercatInputs );
	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
            REGISTER_EVENT(EcatData, ecatData);
		}

        void ecatData(Receiver &receiver)
        {
            // digital in
            std::size_t digitalIn{0};
            receiver.deMarshal(digitalIn);
            for (std::size_t i = 0; i < digitalIn; i++)
            {
                EcatProductIndex productIndex; receiver.deMarshal(productIndex);
                EcatInstance instance; receiver.deMarshal(instance);
                uint8_t value; receiver.deMarshal(value);
                getServer()->ecatDigitalIn(productIndex, instance, value);
            }
            // analog in
            std::size_t analogIn{0};
            receiver.deMarshal(analogIn);
            for (std::size_t i = 0; i < analogIn; i++)
            {
                EcatProductIndex productIndex; receiver.deMarshal(productIndex);
                EcatInstance instance; receiver.deMarshal(instance);
                uint8_t statusCH1; receiver.deMarshal(statusCH1);
                uint16_t valueCH1; receiver.deMarshal(valueCH1);
                uint8_t statusCH2; receiver.deMarshal(statusCH2);
                uint16_t valueCH2; receiver.deMarshal(valueCH2);
                getServer()->ecatAnalogIn(productIndex, instance, statusCH1, valueCH1, statusCH2, valueCH2);
            }
            // oversampling
            std::size_t oversampling{0};
            receiver.deMarshal(oversampling);
            for (std::size_t i = 0; i < oversampling; i++)
            {
                EcatProductIndex productIndex; receiver.deMarshal(productIndex);
                EcatInstance instance; receiver.deMarshal(instance);
                std::size_t size; receiver.deMarshal(size);

                stdVecINT16 data;
                data.reserve(size);
                for(uint8_t i = 0;i < size;i++)
                {
                    uint16_t value; receiver.deMarshal(value);
                    data.push_back(value);
                }
                getServer()->ecatAnalogOversamplingInCH1(productIndex, instance, size, data);

                receiver.deMarshal(size);
                data.clear();
                data.reserve(size);
                for(uint8_t i = 0;i < size;i++)
                {
                    uint16_t value; receiver.deMarshal(value);
                    data.push_back(value);
                }
                getServer()->ecatAnalogOversamplingInCH2(productIndex, instance, size, data);
            }
            // gateway
            std::size_t gateway{0};
            receiver.deMarshal(gateway);
            for (std::size_t i = 0; i < gateway; i++)
            {
                EcatProductIndex productIndex; receiver.deMarshal(productIndex);
                EcatInstance instance; receiver.deMarshal(instance);
                std::size_t size; receiver.deMarshal(size);

                stdVecUINT8 data;
                data.reserve(size);
                for(uint8_t i = 0;i < size;i++)
                {
                    uint8_t value; receiver.deMarshal(value);
                    data.push_back(value);
                }
                getServer()->ecatGatewayIn(productIndex, instance, size, data);
            }
            // encoder
            std::size_t encoder{0};
            receiver.deMarshal(encoder);
            for (std::size_t i = 0; i < encoder; i++)
            {
                EcatProductIndex productIndex; receiver.deMarshal(productIndex);
                EcatInstance instance; receiver.deMarshal(instance);
                uint16_t status; receiver.deMarshal(status);
                uint32_t counterValue; receiver.deMarshal(counterValue);
                uint32_t latchValue; receiver.deMarshal(latchValue);
                getServer()->ecatEncoderIn(productIndex, instance, status, counterValue, latchValue);
            }
            // axis
            std::size_t axis{0};
            receiver.deMarshal(axis);
            for (std::size_t i = 0; i < axis; i++)
            {
                EcatProductIndex productIndex; receiver.deMarshal(productIndex);
                EcatInstance instance; receiver.deMarshal(instance);
                EcatAxisInput axisInput; receiver.deMarshal(axisInput);
                getServer()->ecatAxisIn(productIndex, instance, axisInput);
            }
            // lwm
            std::size_t lwm{0};
            receiver.deMarshal(lwm);
            for (std::size_t i = 0; i < lwm; i++)
            {
                EcatProductIndex productIndex; receiver.deMarshal(productIndex);
                EcatInstance instance; receiver.deMarshal(instance);
                std::size_t size; receiver.deMarshal(size);

                stdVecUINT16 data;
                data.reserve(size);
                for(uint8_t i = 0;i < size;i++)
                {
                    uint16_t value; receiver.deMarshal(value);
                    data.push_back(value);
                }
                getServer()->ecatLWMCh1PlasmaIn(productIndex, instance, size, data);

                receiver.deMarshal(size);
                data.clear();
                data.reserve(size);
                for(uint8_t i = 0;i < size;i++)
                {
                    uint16_t value; receiver.deMarshal(value);
                    data.push_back(value);
                }
                getServer()->ecatLWMCh2TempIn(productIndex, instance, size, data);

                receiver.deMarshal(size);
                data.clear();
                data.reserve(size);
                for(uint8_t i = 0;i < size;i++)
                {
                    uint16_t value; receiver.deMarshal(value);
                    data.push_back(value);
                }
                getServer()->ecatLWMCh3BackRefIn(productIndex, instance, size, data);

                receiver.deMarshal(size);
                data.clear();
                data.reserve(size);
                for(uint8_t i = 0;i < size;i++)
                {
                    uint16_t value; receiver.deMarshal(value);
                    data.push_back(value);
                }
                getServer()->ecatLWMCh4AnalogIn(productIndex, instance, size, data);
            }
        }

	private:
		TEthercatInputs<EventServer> * getServer()
		{
			return server_;
		}

	};

} // namespace interface
} // namespace precitec

#endif /* ETHERCATINPUTS_HANDLER_H_ */

