#ifndef ETHERCATINPUTSSERVER_H_
#define ETHERCATINPUTSSERVER_H_

#include "event/ethercatInputs.server.h"
#include "WeldingHeadControl.h"

namespace precitec
{

namespace ethercat
{

/**
 * EthercatInputsServer
 **/
class EthercatInputsServer : public TEthercatInputs<EventServer>
{
	public:
		EthercatInputsServer(WeldingHeadControl& p_rWeldingHeadControl);
		virtual ~EthercatInputsServer();

		void ecatDigitalIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t value) override;
		void ecatAnalogIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t statusCH1, uint16_t valueCH1, uint8_t statusCH2, uint16_t valueCH2) override;
		void ecatAnalogOversamplingInCH1(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data) override;
		void ecatAnalogOversamplingInCH2(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data) override;
		void ecatGatewayIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT8 &data) override;
		void ecatEncoderIn(EcatProductIndex productIndex, EcatInstance instance, uint16_t status, uint32_t counterValue, uint32_t latchValue) override;
		void ecatAxisIn(EcatProductIndex productIndex, EcatInstance instance, const EcatAxisInput &axisInput) override;
		void ecatLWMCh1PlasmaIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data) override;
		void ecatLWMCh2TempIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data) override;
		void ecatLWMCh3BackRefIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data) override;
		void ecatLWMCh4AnalogIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data) override;

	private:
		WeldingHeadControl &m_rWeldingHeadControl;
};

} // namespace ethercat

} // namespace precitec

#endif // ETHERCATINPUTSSERVER_H_

