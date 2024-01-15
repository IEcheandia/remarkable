#include <stdlib.h>
#include <unistd.h>

#include <cstring>
#include <cmath>
#include <climits>

#include "viWeldHead/ZCollimatorV2/TML_lib_light.h"
//#include "viWeldHead/ZCollimatorV2/variables.h"
#include "viWeldHead/ZCollimatorV2/TmlCommunicator.h"
#include "module/moduleLogger.h"

namespace precitec
{

namespace tml
{

TmlCommunicator::TmlCommunicator():
    m_oTickPerMM(217.30154936),
    m_oTmlConnector(precitec::tml::TmlConnector::instance())
{
}

TmlCommunicator::~TmlCommunicator()
{
}

bool TmlCommunicator::connectTmlController()
{
    const auto success {m_oTmlConnector.connect()};
    return success;
}

bool TmlCommunicator::disconnectTmlController()
{
    const auto success {m_oTmlConnector.disconnect()};
    return success;
}

void TmlCommunicator::SetTmlControllerIpAddress(std::string p_oIpAddress)
{
    m_oTmlConnector.SetTmlControllerIpAddress(p_oIpAddress);
}

bool TmlCommunicator::driveTo(double oNewPosUm, char* p_pPrintString)
{
    int32_t desiredPosition = std::round((oNewPosUm / 1000.0) * TickPerMM()) * (-1); // convert from um to IU and switch sign
    if(desiredPosition > -200)
    {
        desiredPosition = -200;
        wmLogTr( eError, "QnxMsg.VI.ZCollMaxPosInt", "position of Z-collimator is limited due to max. possible position ! (to %d IU)\n", desiredPosition);
    }
    if(desiredPosition < -3648)
    {
        desiredPosition = -3648;
        wmLogTr( eError, "QnxMsg.VI.ZCollMinPosInt", "position of Z-collimator is limited due to min. possible position ! (to %d IU)\n", desiredPosition);
    }

    if (p_pPrintString != nullptr)
    {
        sprintf(p_pPrintString, "driveTo: Value: %d IU", desiredPosition);
    }

    if (!TS_Write32bitValue(1, CPOS_ADDR, desiredPosition))
    {
        wmLog(eDebug, "unable to write register CPOS\n");
        wmLogTr(eError, "QnxMsg.VI.ZCollWriteError", "Problem while writing data to Z-collimator controller %s\n", "(001)");
        return false;
    }
    if (!TS_UpdateImmediate(1))
    {
        wmLog(eDebug, "unable to do TS_UpdateImmediate\n");
        wmLogTr(eError, "QnxMsg.VI.ZCollError", "Error while travelling of Z-collimator\n");
        return false;
    }

    return true;
}

double TmlCommunicator::getActualPositionUm(char* p_pPrintString)
{
    int32_t oPosIU {0};
    if (!TS_Read32bitValue(1, APOS_ADDR, &oPosIU ))
    {
        wmLog(eDebug, "unable to read register APOS\n");
        wmLogTr(eError, "QnxMsg.VI.ZCollReadError", "Problem while reading data from Z-collimator controller %s\n", "(001)");
        return 0.0;
    }
    double oPosUm = (static_cast<double>(oPosIU) / TickPerMM() * 1000.0) * (-1.0); // convert from IU to um and switch sign

    if (p_pPrintString != nullptr)
    {
        sprintf(p_pPrintString, "position of axis: %5d IU %6.3f um", oPosIU, oPosUm);
    }

    return oPosUm;
}

bool TmlCommunicator::doZCollHoming(void)
{
    //if (!TS_StartFunction(1, HOMINGTOMIDDLE)) // alternative way to start homing
    if (!TS_StartFunctionByNumber(1, 1))
    {
        wmLog(eDebug, "unable to start function HOMINGTOMIDDLE\n");
        wmLogTr(eError, "QnxMsg.VI.ZCollErrHoming", "Error while doing reference travelling of Z-collimator\n");
        return false;
    }
    return true;
}

bool TmlCommunicator::readStatus(uint16_t& p_rSRLRegister, uint16_t& p_rSRHRegister, uint16_t& p_rMERRegister, char* p_pPrintString)
{
    if (!TS_ReadStatus(1, REG_SRL, &p_rSRLRegister))
    {
        wmLog(eDebug, "unable to read status register SRL\n");
        wmLogTr(eError, "QnxMsg.VI.ZCollReadError", "Problem while reading data from Z-collimator controller %s\n", "(002)");
        return false;
    }
    if (!TS_ReadStatus(1, REG_SRH, &p_rSRHRegister))
    {
        wmLog(eDebug, "unable to read status register SRH\n");
        wmLogTr(eError, "QnxMsg.VI.ZCollReadError", "Problem while reading data from Z-collimator controller %s\n", "(003)");
        return false;
    }
    if (!TS_ReadStatus(1, REG_MER, &p_rMERRegister))
    {
        wmLog(eDebug, "unable to read status register MER\n");
        wmLogTr(eError, "QnxMsg.VI.ZCollReadError", "Problem while reading data from Z-collimator controller %s\n", "(004)");
        return false;
    }
    if (!TS_ResetFault(1))
    {
        wmLog(eDebug, "unable to reset fault\n");
        wmLogTr(eError, "QnxMsg.VI.ZCollReadError", "Problem while reading data from Z-collimator controller %s\n", "(005)");
        return false;
    }

    if (p_pPrintString != nullptr)
    {
        sprintf(p_pPrintString, "SRH: 0x%04X SRL: 0x%04X MER: 0x%04X", p_rSRHRegister, p_rSRLRegister, p_rMERRegister);
    }

    return true;
}

double TmlCommunicator::TickPerMM(void)
{
    return m_oTickPerMM;
}

std::vector<unsigned char> TmlCommunicator::applicationId(void)
{
    std::vector<std::int16_t> applicationIdAsInts;
    std::int16_t value {0};
    for (std::size_t i = 0; i < 15; ++i)
    {
        // reading 15 consecutive blocks of two bytes each to a std::int16_t.
        TS_Read16bitValue(1, 0x5FCF+i, &value);
        {
            applicationIdAsInts.emplace_back(value);
        }
    }

    auto toUnsignedCharVector = [](const std::vector<std::int16_t>& input)
    {
        static_assert(sizeof(std::int16_t) == 2, "std::int16_t must have size 2.");
        std::vector<unsigned char> result;
        result.reserve(2*input.size());
        for (auto& integer : input)
        {
            if (integer == 0)
            {
                // reached end of application id.
                break;
            }
            unsigned char bytes[2];
            std::copy(reinterpret_cast<const unsigned char*>(&integer), reinterpret_cast<const unsigned char*>(&integer) + 2, bytes);
            result.emplace_back(bytes[0]);
            result.emplace_back(bytes[1]);
        }
        return result;
    };

    const auto applicationIdAsChars = toUnsignedCharVector(applicationIdAsInts);
    return applicationIdAsChars;
}

} // namespace tml
} // namespace precitec

