#include "viWeldHead/Scanlab/RTC6Control.h"

#include "module/moduleLogger.h"

#include <iostream>

namespace RTC6
{
using precitec::Axis;
using precitec::eAxis;
using precitec::eDebug;
using precitec::eError;
using precitec::eWarning;
using precitec::hardware::AnalogOutput;
using precitec::hardware::LaserDelays;
using precitec::hardware::ScannerGeometricalTransformationData;
using precitec::interface::LensType;
using precitec::interface::ScannerModel;
using precitec::PositionDifferenceTolerance;
using precitec::ScanlabCodes;
using precitec::hardware::ScannerControlStatus;
using precitec::ScannerHead;
using precitec::interface::CorrectionFileMode;

namespace
{
static const std::string RTC6_IP_ADDRESS{"192.168.170.105"};
static const std::string RTC6_IP_NETMASK{"255.255.255.0"};

static constexpr unsigned int RTC6_NUMBER_OF_LIST_COMMANDS{250000};
static constexpr unsigned int RTC6_LIST_1_MEMORY{RTC6_NUMBER_OF_LIST_COMMANDS};
static constexpr unsigned int RTC6_LIST_2_MEMORY{RTC6_NUMBER_OF_LIST_COMMANDS};
static constexpr unsigned int RTC6_NUMBER_OF_INFORMATION_FOR_ONE_POINT{5};
static constexpr unsigned int RTC6_NUMBER_OF_OVERHEAD_COMMANDS_FOR_CONTOUR{100};

void ScannerCheckError(uint32_t error)
{
    if ((error & 0x01))
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabRTC6NoCard", "Scanlab RTC6 control has a failure: No RTC6 card found, 0x%x\n", (error & 0x01));
    }
    if ((error & 0x02))
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabRTC6AccessDenied", "Scanlab RTC6 control has a failure: Access denied, 0x%x\n", (error & 0x02));
    }
    if ((error & 0x04))
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabRTC6CommandNotSent", "Scanlab RTC6 control has a failure: Command not sent, 0x%x\n", (error & 0x04));
    }
    if ((error & 0x08))
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabRTC6NoRespond", "Scanlab RTC6 control has a failure: RTC6 card doesn't respond, 0x%x\n", (error & 0x08));
    }
    if ((error & 0x10))
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabRTC6InvalidParameter", "Scanlab RTC6 control has a failure: Invalid parameters, 0x%x\n", (error & 0x10));
    }
    if ((error & 0x20))
    {
        wmLogTr(eWarning, "QnxMsg.VI.ScanlabRTC6Busy", "Scanlab RTC6 control has a warning: RTC6 Card is busy, 0x%x, \n", (error & 0x20));
    }
    if ((error & 0x40))
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabRTC6Rejected", "Scanlab RTC6 control has a failure: List command rejected, 0x%x\n", (error & 0x40));
    }
    if ((error & 0x80))
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabRTC6Ignored", "Scanlab RTC6 control has a failure: List command ignored, 0x%x\n", (error & 0x80));
    }
    if ((error & 0x100))
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabRTC6VersionMismatch", "Scanlab RTC6 control has a failure: RTC6 version mismatch, 0x%x\n", (error & 0x100));
    }
    if ((error & 0x200))
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabRTC6CorruptedDownload", "Scanlab RTC6 control has a failure: Corrupted download, 0x%x\n", (error & 0x200));
    }
    if ((error & 0x400))
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabRTC6TypeRejected", "Scanlab RTC6 control has a failure: RTC6 command type rejected, 0x%x\n", (error & 0x400));
    }
    if ((error & 0x800))
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabRTC6Windows", "Scanlab RTC6 control has a failure: Windows memory request failed, 0x%x\n", (error & 0x800));
    }
    if ((error & 0x1000))
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabRTC6Download", "Scanlab RTC6 control has a failure: Download error, 0x%x\n", (error & 0x1000));
    }
    if ((error & 0x2000))
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabRTC6Ethernet", "Scanlab RTC6 control has a failure: Ethernet error, 0x%x\n", (error & 0x2000));
    }
    if ((error & 0x8000))
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabRTC6WindowsVersion", "Scanlab RTC6 control has a failure: Windows version not supported, 0x%x\n", (error & 0x8000));
    }
    if ((error & 0x10000))
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabRTC6Config", "Scanlab RTC6 control has a failure: Config error, 0x%x\n", (error & 0x10000));
    }
}

void HeadCheckError(uint32_t error)
{
    if ((error & 0x08) == 0)
    {
        wmLogTr(eError, "QnxMsg.VI.ScanlabHeadStatusXAxis", "Scanner x position feedback not OK, target position not reached: %x\n", error);
    }
    if ((error & 0x10) == 0)
    {
        wmLogTr(eError, "QnxMsg.VI.ScanlabHeadStatusYAxis", "Scanner y position feedback not OK, target position not reached: %x\n", error);
    }
    if ((error & 0x40) == 0)
    {
        wmLogTr(eWarning, "QnxMsg.VI.ScanlabHeadStatusTemp", "Scanner temperature not in optimal range: %x\n", error);
    }
    if ((error & 0x80) == 0)
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabHeadStatusPower", "Scanner power not OK or temperature in critical range: %x\n", error);
    }
}
}

Control::Control(LensType lensType)
    : m_board{RTC6_IP_ADDRESS, RTC6_IP_NETMASK, RTC6_LIST_1_MEMORY, RTC6_LIST_2_MEMORY}
    , m_scanner{0, 0, 0.0, 0, 0, 0}
    , m_laser{3, 1, 0, 4095, 0, 0} //4095 for one, 0x10001000 for both ports at the same time
{
}

ScannerControlStatus Control::init(std::string ipAddress, const std::string& scanlabCorrectionFile, LensType lensType, ScannerModel scannerModel)
{
    m_board.set_RTCConnectInfo(std::move(ipAddress), RTC6_IP_NETMASK);
    if (m_board.init() != 0)
    {
        //        m_oScanlabScannerEnable = false;
        wmFatal(eAxis, "QnxMsg.VI.ScanlabNoRT6", "There is no Scanlab RTC6 control available (%s)\n", "001");
        return ScannerControlStatus::InitializationFailed;
    }
    uint32_t oDLLVersion;
    uint32_t oBIOSVersion;
    uint32_t oHexVersion;
    uint32_t oRTCVersion;
    uint32_t oSerialNumber;
    m_board.GetVersionInfos(oDLLVersion, oBIOSVersion, oHexVersion, oRTCVersion, oSerialNumber);
    wmLog(eDebug, "Scanlab DLL version:   %d\n", oDLLVersion);
    wmLog(eDebug, "Scanlab BIOS version:  %x\n", oBIOSVersion);
    if (oHexVersion >= 3000)
    {
        wmLog(eDebug, "Scanlab HEX version:   %d + 3D option\n", (oHexVersion - 3000));
    }
    else
    {
        wmLog(eDebug, "Scanlab HEX version:   %d\n", oHexVersion);
    }
    wmLog(eDebug, "Scanlab RTC version:   %d\n", ((oRTCVersion & 0xFF) + 600));
    wmLog(eDebug, "Scanlab RTC options:   0x%x\n", (oRTCVersion & ~0xFF));
    wmLog(eDebug, "Scanlab serial number: %d\n", oSerialNumber);

    m_scanner.setCustomCorrectionFile(scanlabCorrectionFile);
    m_scanner.setLensType(lensType);
    m_scanner.setScannerModel(scannerModel);

    if (m_scanner.init())
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabNoRT6", "There is no Scanlab RTC6 control available (%s)\n", "002");
        return ScannerControlStatus::InitializationFailed;
    }

    wmLog(eDebug, "Scanlab correction file: %s\n", m_scanner.correctionFile());

    m_laser.setScannerModel(scannerModel);
    m_laser.init();

    return ScannerControlStatus::Initialized;
}

double Control::calibValueBitsPerMM()
{
    return m_scanner.get_CalibValueBitsPerMM();
}

unsigned int Control::numberOfPointsFromContour(std::size_t contourSize)
{
    return contourSize / RTC6_NUMBER_OF_INFORMATION_FOR_ONE_POINT;
}

unsigned int Control::numberOfPossiblePointsForListMemory()
{
    return RTC6_NUMBER_OF_LIST_COMMANDS / (RTC6_NUMBER_OF_INFORMATION_FOR_ONE_POINT - 1) - RTC6_NUMBER_OF_OVERHEAD_COMMANDS_FOR_CONTOUR;
}

void Control::setLaserDelays(LaserDelays delays)
{
    m_laser.set_LaserDelays(delays.on, delays.off);
}

void Control::setScannerJumpSpeed(double speedInMeterPerSec)
{
    m_scanner.SetJumpSpeed(calculateJumpSpeed(speedInMeterPerSec));
    scannerCheckLastError();
    const uint32_t errorNumber = m_scanner.GetHeadStatus(1);
    HeadCheckError(errorNumber);
}

void Control::setScannerMarkSpeed(double speedInMeterPerSec)
{
    m_scanner.SetMarkSpeed(calculateMarkSpeed(speedInMeterPerSec));
    scannerCheckLastError();
    const uint32_t errorNumber = m_scanner.GetHeadStatus(1);
    HeadCheckError(errorNumber);
}


double Control::calculateJumpSpeed(double meterPerSeconds) const
{
    return calculateMarkSpeed(meterPerSeconds);
}

double Control::calculateMarkSpeed(double meterPerSeconds) const
{
    double speedInBitsPerMs = meterPerSeconds * m_scanner.get_CalibValueBitsPerMM();
    if (speedInBitsPerMs < 1.6)
    {
        speedInBitsPerMs = 1.6;
    }
    if (speedInBitsPerMs > 800000.0)
    {
        speedInBitsPerMs = 800000.0;
    }
    return speedInBitsPerMs;
}

void Control::sendToleranceInBitsToScanner(int toleranceInBits)
{
    uint32_t commandWithTolerance = static_cast<int>(PositionDifferenceTolerance::ToleranceCommand) + toleranceInBits; //5376 = 0x1500; 0x15XX --> XX = tolerance from 01 (0,0015%) to FF (0,3825%); default: B7 = 183 = 0,28%

    m_scanner.ReadControlValue(ScannerHead::ScannerHead1, Axis::Axis1, commandWithTolerance);
    m_scanner.ReadControlValue(ScannerHead::ScannerHead1, Axis::Axis2, commandWithTolerance);
}

void Control::scannerDriveToZero()
{
    m_scanner.GotoXY(0, 0);
    scannerCheckLastError();
    uint32_t errorNumber = m_scanner.GetHeadStatus(1);
    HeadCheckError(errorNumber);
}

void Control::scannerDriveToPosition(double xPosInMM, double yPosInMM)
{
    const int32_t xPosInBits = static_cast<int32_t>(xPosInMM * m_scanner.get_CalibValueBitsPerMM());
    const int32_t yPosInBits = static_cast<int32_t>(yPosInMM * m_scanner.get_CalibValueBitsPerMM());
    wmLog(eDebug, "Scanlab::ScannerDriveToPosition (%d,%d)\n", xPosInBits, yPosInBits);

    m_scanner.GotoXY(xPosInBits, yPosInBits);
    scannerCheckLastError();
    uint32_t errorNumber = m_scanner.GetHeadStatus(1);
    HeadCheckError(errorNumber);
}

void Control::scannerDriveWithOCTReference(int32_t xPosInBits, int32_t yPosInBits, uint16_t binaryValue, uint16_t maskValue)
{
    m_scanner.DriveWithOCTReferenceControl(xPosInBits, yPosInBits, binaryValue, maskValue);
    scannerCheckLastError();
    uint32_t errorNumber = m_scanner.GetHeadStatus(1);
    HeadCheckError(errorNumber);
}

void Control::scannerSetOCTReference(uint16_t binaryValue, uint16_t maskValue)
{
    m_scanner.SetOCTReferenceControl(binaryValue, maskValue);
    scannerCheckLastError();
    uint32_t errorNumber = m_scanner.GetHeadStatus(1);
    HeadCheckError(errorNumber);
}

void Control::jumpScannerOffset()
{
    m_scanner.jump_ScannerOffset();
}

void Control::checkStatusWord(precitec::Axis axis)
{
    int axisInInt = static_cast<int>(axis);
    int code = static_cast<int>(ScanlabCodes::StatusWordCode);
    int32_t statusWordFeedbackCode = m_scanner.ReadControlValue(1, axisInInt, code);
    if ((statusWordFeedbackCode & 0x000FFFFF) != 0x000FDFD0)
    {
        wmLogTr(eWarning, "QnxMsg.VI.ScanlabStatusNOK", "Status word of axis %d has a warning: 0x%x\n", axisInInt, statusWordFeedbackCode);
    }
}

void Control::checkTemperature(precitec::Axis axis)
{
    int axisInInt = static_cast<int>(axis);
    int code = static_cast<int>(ScanlabCodes::TemperatureCode);
    int32_t statusWordFeedbackCode = m_scanner.ReadControlValue(1, axisInInt, code);
    float oTemperature = static_cast<float>(statusWordFeedbackCode) / 160.0;
    if (oTemperature > 50.0)
    {
        wmLogTr(eWarning, "QnxMsg.VI.ScanlabTempWarn", "Scanlab scanner axis %d: temperature warning (%s)\n", axisInInt, "001");
    }
}

void Control::checkServoCardTemperature(precitec::Axis axis)
{
    int axisInInt = static_cast<int>(axis);
    int code = static_cast<int>(ScanlabCodes::ServoCardTemperatureCode);
    int32_t statusWordFeedbackCode = m_scanner.ReadControlValue(1, axisInInt, code);
    float oTemperature = static_cast<float>(statusWordFeedbackCode) / 160.0;
    if (oTemperature > 50.0)
    {
        wmLogTr(eWarning, "QnxMsg.VI.ScanlabTempWarn", "Scanlab scanner axis %d: temperature warning (%s)\n", axisInInt, "002");
    }
}

void Control::checkStopEvent(precitec::Axis axis)
{
    int axisInInt = static_cast<int>(axis);
    int code = static_cast<int>(ScanlabCodes::StoppEventCode);
    int32_t stopEventFeedbackCode = m_scanner.ReadControlValue(1, axisInInt, code);
    if (stopEventFeedbackCode != 0) // scanner is stopped, there is a failure
    {
        wmFatal(eAxis, "QnxMsg.VI.ScanlabError", "Scanlab scanner axis %d has a failure: 0x%x\n", axisInInt, stopEventFeedbackCode);
    }
}

void Control::scannerCheckLastError()
{
    uint32_t oLastError = 0;
    uint32_t oError = 0;
    m_scanner.GetScannerError(oError, oLastError);
    ScannerCheckError(oError);
    ScannerCheckError(oLastError);
}

void Control::scannerResetLastError()
{
    m_scanner.ResetScannerError(); // clear the error register
}

void Control::scannerTestFunction1()
{
#if 0
    int32_t oValue = m_oScanner.ReadControlValue(1, 1, 0x0501); // Achse 1 Ist-Position (Winkelstellung)
    std::cout << "---> get_value: " << oValue << " (Achse 1 Ist-Position)" << std::endl;
    ScannerCheckLastError();

    oValue = m_oScanner.ReadControlValue(1, 2, 0x0501); // Achse 2 Ist-Position (Winkelstellung)
    std::cout << "---> get_value: " << oValue << " (Achse 2 Ist-Position)" << std::endl;
    ScannerCheckLastError();
#endif

    int32_t oValue = m_scanner.ReadControlValue(1, 1, 0x0528); // Achse 1 Betriebszustand
    std::cout << "---> get_value: 0x" << std::hex << oValue << std::dec << " (Achse 1 Betriebszustand)" << std::endl;
    scannerCheckLastError();

    oValue = m_scanner.ReadControlValue(1, 2, 0x0528); // Achse 2 Betriebszustand
    std::cout << "---> get_value: 0x" << std::hex << oValue << std::dec << " (Achse 2 Betriebszustand)" << std::endl;
    scannerCheckLastError();

    oValue = m_scanner.ReadControlValue(1, 1, 0x052A); // Achse 1 Stopp-Ereigniscode
    std::cout << "---> get_value: 0x" << std::hex << oValue << std::dec << " (Achse 1 Stopp-Ereigniscode)" << std::endl;
    scannerCheckLastError();

    oValue = m_scanner.ReadControlValue(1, 2, 0x052A); // Achse 2 Stopp-Ereigniscode
    std::cout << "---> get_value: 0x" << std::hex << oValue << std::dec << " (Achse 2 Stopp-Ereigniscode)" << std::endl;
    scannerCheckLastError();

    oValue = m_scanner.ReadControlValue(1, 1, 0x0500); // Achse 1 Statuswort
    std::cout << "---> get_value: 0x" << std::hex << oValue << std::dec << " (Achse 1 Statuswort)" << std::endl;
    scannerCheckLastError();

    oValue = m_scanner.ReadControlValue(1, 2, 0x0500); // Achse 2 Statuswort
    std::cout << "---> get_value: 0x" << std::hex << oValue << std::dec << " (Achse 2 Statuswort)" << std::endl;
    scannerCheckLastError();

    std::cout << std::endl;
}

void Control::scannerTestFunction2()
{
    int32_t oValue = m_scanner.ReadControlValue(1, 1, 0x0514); // Achse 1 Temperatur Galvanometerscanner
    std::cout << "---> get_value: " << static_cast<float>(oValue) / 160.0 << " (Achse 1 Temperatur Galvo)" << std::endl;
    scannerCheckLastError();

    oValue = m_scanner.ReadControlValue(1, 2, 0x0514); // Achse 2 Temperatur Galvanometerscanner
    std::cout << "---> get_value: " << static_cast<float>(oValue) / 160.0 << " (Achse 2 Temperatur Galvo)" << std::endl;
    scannerCheckLastError();

    oValue = m_scanner.ReadControlValue(1, 1, 0x0515); // Achse 1 Temperatur Servokarte
    std::cout << "---> get_value: " << static_cast<float>(oValue) / 160.0 << " (Achse 1 Temperatur Servo)" << std::endl;
    scannerCheckLastError();

    oValue = m_scanner.ReadControlValue(1, 2, 0x0515); // Achse 2 Temperatur Servokarte
    std::cout << "---> get_value: " << static_cast<float>(oValue) / 160.0 << " (Achse 2 Temperatur Servo)" << std::endl;
    scannerCheckLastError();

    oValue = m_scanner.ReadControlValue(1, 1, 0x0518); // Achse 1 DSP IO Spannung
    std::cout << "---> get_value: " << oValue << " (Achse 1 DSP IO Spannung)" << std::endl;
    scannerCheckLastError();

    oValue = m_scanner.ReadControlValue(1, 2, 0x0518); // Achse 2 DSP IO Spannung
    std::cout << "---> get_value: " << oValue << " (Achse 2 DSP IO Spannung)" << std::endl;
    scannerCheckLastError();

    std::cout << std::endl;
}

LaserDelays Control::getLaserDelays() const
{
    long int onDelay = 0;
    long int offDelay = 0;
    m_laser.get_LaserDelays(onDelay, offDelay);
    return {onDelay, offDelay};
}

AnalogOutput Control::getAnalogOutput() const
{
    unsigned int value, minValue, maxValue;
    m_laser.get_AnalogOutput(value, minValue, maxValue);
    return {value, minValue, maxValue};
}

ScannerGeometricalTransformationData Control::getScannerGeometricalTransformationData() const
{
    long int xOffset = 0;
    long int yOffset = 0;
    double angle = 0.;
    m_scanner.get_ScannerOffset(xOffset, yOffset, angle);
    return {xOffset, yOffset, angle};
}

double Control::selectCorrectionFileMode(CorrectionFileMode mode)
{
    return m_scanner.selectCorrectionFile(mode);
}

}
