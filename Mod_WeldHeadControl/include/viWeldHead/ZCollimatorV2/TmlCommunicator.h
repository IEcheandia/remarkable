#pragma once

#include <vector>

#include "viWeldHead/ZCollimatorV2/TmlConnector.h"

namespace precitec
{

namespace tml
{

class TmlCommunicator
{
public:
    TmlCommunicator ();
    virtual ~TmlCommunicator();
    TmlCommunicator (const TmlCommunicator&) = delete;
    void operator=(const TmlCommunicator&) = delete;

    bool connectTmlController(void);
    bool disconnectTmlController(void);
    void SetTmlControllerIpAddress(std::string p_oIpAddress);
    bool driveTo(double oNewPosUm, char* p_pPrintString = nullptr);
    double getActualPositionUm(char* p_pPrintString = nullptr);
    bool doZCollHoming(void);
    bool readStatus(uint16_t& p_rSRLRegister, uint16_t& p_rSRHRegister, uint16_t& p_rMERRegister, char* p_pPrintString = nullptr);

private:
    double TickPerMM(void);
    std::vector<unsigned char> applicationId(void);

    const double m_oTickPerMM;
    precitec::tml::TmlConnector& m_oTmlConnector;
};

} // namespace tml
} // namespace precitec

