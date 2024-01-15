/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       19.11.2016
 *  @brief      Part of the EthercatInputs Interface
 *  @details
 */

#ifndef ETHERCATINPUTS_SERVER_H_
#define ETHERCATINPUTS_SERVER_H_

#include "event/ethercatInputs.interface.h"

namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <>
	class TEthercatInputs<EventServer> : public TEthercatInputs<AbstractInterface>
	{
	public:
		TEthercatInputs(){}
		virtual ~TEthercatInputs() {}
	public:
		/// interface EthercatInputs : ecatDigitalIn
		virtual void ecatDigitalIn (EcatProductIndex productIndex, EcatInstance instance, uint8_t value) {}
		/// interface EthercatInputs : ecatAnalogIn
		virtual void ecatAnalogIn (EcatProductIndex productIndex, EcatInstance instance, uint8_t statusCH1, uint16_t valueCH1, uint8_t statusCH2, uint16_t valueCH2) {}
		/// interface EthercatInputs : ecatAnalogOversamplingInCH1
		virtual void ecatAnalogOversamplingInCH1 (EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data) {}
		/// interface EthercatInputs : ecatAnalogOversamplingInCH2
		virtual void ecatAnalogOversamplingInCH2 (EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data) {}
		/// interface EthercatInputs : ecatGatewayIn
		virtual void ecatGatewayIn (EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT8 &data) {}
		/// interface EthercatInputs : ecatEncoderIn
		virtual void ecatEncoderIn (EcatProductIndex productIndex, EcatInstance instance, uint16_t status, uint32_t counterValue, uint32_t latchValue) {}
		/// interface EthercatInputs : ecatAxisIn
		virtual void ecatAxisIn (EcatProductIndex productIndex, EcatInstance instance, const EcatAxisInput &axisInput) {}
		/// interface EthercatInputs : ecatLWMCh1PlasmaIn
		virtual void ecatLWMCh1PlasmaIn (EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data) {}
		/// interface EthercatInputs : ecatLWMCh2TempIn
		virtual void ecatLWMCh2TempIn (EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data) {}
		/// interface EthercatInputs : ecatLWMCh3BackRefIn
		virtual void ecatLWMCh3BackRefIn (EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data) {}
		/// interface EthercatInputs : ecatLWMCh4AnalogIn
		virtual void ecatLWMCh4AnalogIn (EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data) {}

    private:
		void ecatData(const EtherCAT::EcatInData &data) override {}

	};


} // namespace interface
} // namespace precitec

#endif /* ETHERCATINPUTS_SERVER_H_ */

