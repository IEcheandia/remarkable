/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       23.11.2016
 *  @brief      Part of the EthercatOutputs Interface
 *  @details
 */

#ifndef ETHERCATOUTPUTS_SERVER_H_
#define ETHERCATOUTPUTS_SERVER_H_

#include "event/ethercatOutputs.interface.h"

namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <>
	class TEthercatOutputs<EventServer> : public TEthercatOutputs<AbstractInterface>
	{
	public:
		TEthercatOutputs(){}
		virtual ~TEthercatOutputs() {}
	public:
		/// interface EthercatOutputs : ecatDigitalOut
		virtual void ecatDigitalOut (EcatProductIndex productIndex, EcatInstance instance, uint8_t value, uint8_t mask) {}
		/// interface EthercatOutputs : ecatAnalogOut
		virtual void ecatAnalogOut (EcatProductIndex productIndex, EcatInstance instance, EcatChannel channel, uint16_t value) {}
		/// interface EthercatOutputs : ecatGatewayOut
		virtual void ecatGatewayOut (EcatProductIndex productIndex, EcatInstance instance, uint8_t size, stdVecUINT8 data, stdVecUINT8 mask) {}
		/// interface EthercatOutputs : ecatEncoderOut
		virtual void ecatEncoderOut (EcatProductIndex productIndex, EcatInstance instance, uint16_t command, uint32_t setCounterValue) {}
		/// interface EthercatOutputs : ecatAxisOut
		virtual void ecatAxisOut (EcatProductIndex productIndex, EcatInstance instance, EcatAxisOutput axisOutput) {}
		/// interface EthercatOutputs : ecatRequestSlaveInfo
		virtual void ecatRequestSlaveInfo (void) {}
		/// interface EthercatOutputs : ecatFRONTENDOut
		virtual void ecatFRONTENDOut (EcatProductIndex productIndex, EcatInstance instance, EcatFRONTENDOutput frontendOutput) {}

	};


} // namespace interface
} // namespace precitec

#endif /* ETHERCATOUTPUTS_SERVER_H_ */

