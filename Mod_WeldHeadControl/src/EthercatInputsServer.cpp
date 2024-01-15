#include "viWeldHead/EthercatInputsServer.h"

namespace precitec
{

namespace ethercat
{

EthercatInputsServer::EthercatInputsServer(WeldingHeadControl& p_rWeldingHeadControl)
	:m_rWeldingHeadControl(p_rWeldingHeadControl)
{
}

EthercatInputsServer::~EthercatInputsServer()
{
}

void EthercatInputsServer::ecatDigitalIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t value)
{
	m_rWeldingHeadControl.ecatDigitalIn(productIndex, instance, value);
}

void EthercatInputsServer::ecatAnalogIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t statusCH1, uint16_t valueCH1, uint8_t statusCH2, uint16_t valueCH2)
{
	m_rWeldingHeadControl.ecatAnalogIn(productIndex, instance, statusCH1, valueCH1, statusCH2, valueCH2);
}

void EthercatInputsServer::ecatAnalogOversamplingInCH1(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data)
{
	m_rWeldingHeadControl.ecatAnalogOversamplingInCH1(productIndex, instance, size, data);
}

void EthercatInputsServer::ecatAnalogOversamplingInCH2(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data)
{
	m_rWeldingHeadControl.ecatAnalogOversamplingInCH2(productIndex, instance, size, data);
}

void EthercatInputsServer::ecatGatewayIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT8 &data)
{
}

void EthercatInputsServer::ecatEncoderIn(EcatProductIndex productIndex, EcatInstance instance, uint16_t status, uint32_t counterValue, uint32_t latchValue)
{
	m_rWeldingHeadControl.ecatEncoderIn(productIndex, instance, status, counterValue, latchValue);
}

void EthercatInputsServer::ecatAxisIn(EcatProductIndex productIndex, EcatInstance instance, const EcatAxisInput &axisInput)
{
    if (m_rWeldingHeadControl.getStatesHeadX() != nullptr)
    {
        if (m_rWeldingHeadControl.getInstanceAxisX() == (int)instance)
        {
            m_rWeldingHeadControl.getStatesHeadX()->ecatAxisIn(productIndex, instance, axisInput);
        }
    }
    if (m_rWeldingHeadControl.getStatesHeadY() != nullptr)
    {
        if (m_rWeldingHeadControl.getInstanceAxisY() == (int)instance)
        {
            m_rWeldingHeadControl.getStatesHeadY()->ecatAxisIn(productIndex, instance, axisInput);
        }
    }
    if (m_rWeldingHeadControl.getStatesHeadZ() != nullptr)
    {
        if (m_rWeldingHeadControl.getInstanceAxisZ() == (int)instance)
        {
            m_rWeldingHeadControl.getStatesHeadZ()->ecatAxisIn(productIndex, instance, axisInput);
        }
    }
}

void EthercatInputsServer::ecatLWMCh1PlasmaIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data)
{
    m_rWeldingHeadControl.ecatLWMCh1PlasmaIn(productIndex, instance, size, data);
}

void EthercatInputsServer::ecatLWMCh2TempIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data)
{
    m_rWeldingHeadControl.ecatLWMCh2TempIn(productIndex, instance, size, data);
}

void EthercatInputsServer::ecatLWMCh3BackRefIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data)
{
    m_rWeldingHeadControl.ecatLWMCh3BackRefIn(productIndex, instance, size, data);
}

void EthercatInputsServer::ecatLWMCh4AnalogIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data)
{
    m_rWeldingHeadControl.ecatLWMCh4AnalogIn(productIndex, instance, size, data);
}

}	// namespace ethercat

} 	// namespace precitec;

