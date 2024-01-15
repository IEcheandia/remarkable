/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       19.11.2016
 *  @brief      Part of the EthercatInputs Interface
 *  @details
 */

#ifndef ETHERCATINPUTS_PROXY_H_
#define ETHERCATINPUTS_PROXY_H_

#include "event/ethercatInputs.h"
#include "event/ethercatInputs.interface.h"
#include "server/eventProxy.h"

namespace precitec
{

namespace interface
{

	template <>
	class TEthercatInputs<EventProxy> : public Server<EventProxy>, public TEthercatInputs<AbstractInterface>, public TEthercatInputsMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TEthercatInputs() : EVENT_PROXY_CTOR(TEthercatInputs), TEthercatInputs<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

		virtual ~TEthercatInputs() {}

	public:

        void ecatData(const EtherCAT::EcatInData &data) override
        {
            INIT_EVENT(EcatData);
            // digital in
            signaler().marshal(data.digitalIn.size());
            for (const auto &element : data.digitalIn)
            {
                signaler().marshal(element.productIndex);
                signaler().marshal(element.instance);
                signaler().marshal(element.value);
            }
            // analog in
            signaler().marshal(data.analogIn.size());
            for (const auto &element : data.analogIn)
            {
                signaler().marshal(element.productIndex);
                signaler().marshal(element.instance);
                signaler().marshal(element.statusCH1);
                signaler().marshal(element.valueCH1);
                signaler().marshal(element.statusCH2);
                signaler().marshal(element.valueCH2);
            }
            // oversampling
            signaler().marshal(data.oversampling.size());
            for (const auto &element : data.oversampling)
            {
                signaler().marshal(element.productIndex);
                signaler().marshal(element.instance);
                signaler().marshal(element.channel1.size());
                for(const auto &data : element.channel1)
                {
                    signaler().marshal(data);
                }
                signaler().marshal(element.channel2.size());
                for(const auto &data : element.channel2)
                {
                    signaler().marshal(data);
                }
            }
            // gateway
            signaler().marshal(data.gateway.size());
            for (const auto &element : data.gateway)
            {
                signaler().marshal(element.productIndex);
                signaler().marshal(element.instance);
                signaler().marshal(element.data.size());
                for (const auto &data : element.data)
                {
                    signaler().marshal(data);
                }
            }
            // encoder
            signaler().marshal(data.encoder.size());
            for (const auto &element : data.encoder)
            {
                signaler().marshal(element.productIndex);
                signaler().marshal(element.instance);
                signaler().marshal(element.status);
                signaler().marshal(element.counterValue);
                signaler().marshal(element.latchValue);
            }
            // axis
            signaler().marshal(data.axis.size());
            for (const auto &element : data.axis)
            {
                signaler().marshal(element.productIndex);
                signaler().marshal(element.instance);
                signaler().marshal(element.axis);
            }
            // lwm
            signaler().marshal(data.lwm.size());
            for (const auto &element : data.lwm)
            {
                signaler().marshal(element.productIndex);
                signaler().marshal(element.instance);
                signaler().marshal(element.plasma.size());
                for(const auto &data : element.plasma)
                {
                    signaler().marshal(data);
                }
                signaler().marshal(element.temperature.size());
                for(const auto &data : element.temperature)
                {
                    signaler().marshal(data);
                }
                signaler().marshal(element.backReference.size());
                for(const auto &data : element.backReference)
                {
                    signaler().marshal(data);
                }
                signaler().marshal(element.analog.size());
                for(const auto &data : element.analog)
                {
                    signaler().marshal(data);
                }
            }

            signaler().send();
        }

	};

} // namespace interface
} // namespace precitec

#endif /* ETHERCATINPUTS_PROXY_H_ */

