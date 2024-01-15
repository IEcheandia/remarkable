#ifndef ETHERCATINPUTSSERVER_H_
#define ETHERCATINPUTSSERVER_H_

#include "event/ethercatInputs.server.h"
#include "VI_InspectionControl.h"

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

		/**
		 * Ctor.
		 * @param _service Service
		 * @return void
		 **/
		EthercatInputsServer(VI_InspectionControl& p_rInspectionControl);
		virtual ~EthercatInputsServer();

		/**
		 * Kritischer Fehler -> gehe in NotReady Zustand
		 * @param relatedException Ursache fuer notReady Zustand
		 */
		void ecatDigitalIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t value) override
		{
			m_rInspectionControl.ecatDigitalIn(productIndex, instance, value);
		}

		void ecatAnalogIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t statusCH1, uint16_t valueCH1, uint8_t statusCH2, uint16_t valueCH2) override
		{
			m_rInspectionControl.ecatAnalogIn(productIndex, instance, statusCH1, valueCH1, statusCH2, valueCH2);
		}

		void ecatAnalogOversamplingInCH1(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data) override
		{
			m_rInspectionControl.ecatAnalogOversamplingInCH1(productIndex, instance, size, data);
		}

		void ecatAnalogOversamplingInCH2(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data) override
		{
			m_rInspectionControl.ecatAnalogOversamplingInCH2(productIndex, instance, size, data);
		}

		void ecatGatewayIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT8 &data) override
		{
			m_rInspectionControl.ecatGatewayIn(productIndex, instance, size, data);
		}

	private:
		VI_InspectionControl &m_rInspectionControl;
};

} // namespace ethercat

} // namespace precitec

#endif // ETHERCATINPUTSSERVER_H_

