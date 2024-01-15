#include "viWeldHead/Scanlab/RTC6Scanner.h"

#include <iostream>
#include <fstream>
#include "rtc6.h"
#include "viWeldHead/Scanlab/scannerLogger.h"
#include "common/definesScanlab.h"

using precitec::hardware::logger::ScannerLogger;
using precitec::CorrectionFileErrorCode;

static const double INVALID_VALUE{0.0};
static const unsigned int CORRECTION_FILE_DIMENSION_MODE{2};
static const unsigned int DISABLE_SIGNALS_AT_SCANHEAD{0};
static const unsigned int GET_CALIBRATION_FACTOR{1};

namespace RTC6
{
Scanner::Scanner():
    m_oCorrectionFile(""),
    m_correctionFileType(CorrectionFileType::lenseSpecific),
    m_oCalibValueBitsPerMM(4000.0),
    m_oXOffset(0),
    m_oYOffset(0),
    m_oAngle(0),
    m_oJumpDelay(0),
    m_oMarkDelay(0),
    m_oPolygonDelay(0)
{
}

Scanner::Scanner(long int p_oXOffset, long int p_oYOffset, double p_oAngle, long int p_oJumpDelay, long int p_oMarkDelay, long int p_oPolygonDelay):
    m_oCalibValueBitsPerMM(4000.0),
    m_oXOffset(p_oXOffset),
    m_oYOffset(p_oYOffset),
    m_oAngle(p_oAngle),
    m_oJumpDelay(p_oJumpDelay),
    m_oMarkDelay(p_oMarkDelay),
    m_oPolygonDelay(p_oPolygonDelay)
{
}

bool Scanner::init()
{
    checkCustomCorrectionFileSpecified();
    if (auto initFailed = loadCorrectionFiles())
    {
        return initFailed;
    }

    selectCorrectionFile(CorrectionFileMode::Welding);
    applyScannerDefaultSettings();

    return false;
}

double Scanner::selectCorrectionFile(CorrectionFileMode mode)
{
    if (!checkCorrectionFileModeIsSelectable(mode))
    {
        return INVALID_VALUE;
    }
    auto correctionFileNumber = static_cast<int>(mode);
    select_cor_table(correctionFileNumber, DISABLE_SIGNALS_AT_SCANHEAD);
    m_oCalibValueBitsPerMM = get_head_para(precitec::ScannerHead::ScannerHead1, GET_CALIBRATION_FACTOR);
    return m_oCalibValueBitsPerMM;
}

void Scanner::setCustomCorrectionFile(const std::string& customCorrectionFile)
{
    m_customCorrectionFile = customCorrectionFile;
}

void Scanner::setLensType(LensType lens)
{
    m_lensType = lens;
}

void Scanner::set_ScannerOffset(long int p_oXOffset, long int p_oYOffset, double p_oAngle)
{
    m_oXOffset = p_oXOffset;
    m_oYOffset = p_oYOffset;
    m_oAngle = p_oAngle;
}

void Scanner::set_ScannerDelays(long int p_oJumpDelay, long int p_oMarkDelay, long int p_oPolygonDelay)
{
    m_oJumpDelay = p_oJumpDelay;
    m_oMarkDelay = p_oMarkDelay;
    m_oPolygonDelay = p_oPolygonDelay;
}

void Scanner::get_ScannerOffset(long int& p_rXOffset, long int& p_rYOffset, double& p_rAngle) const
{
    p_rXOffset = m_oXOffset;
    p_rYOffset = m_oYOffset;
    p_rAngle = m_oAngle;
}

void Scanner::get_ScannerDelays(long int& p_rJumpDelay, long int& p_rMarkDelay, long int& p_rPolygonDelay)
{
    p_rJumpDelay = m_oJumpDelay;
    p_rMarkDelay = m_oMarkDelay;
    p_rPolygonDelay = m_oPolygonDelay;
}

double Scanner::get_CalibValueBitsPerMM(void) const
{
    return m_oCalibValueBitsPerMM;
}

void Scanner::jump_ScannerOffset()
{
    set_offset(1, m_oXOffset, m_oYOffset, 1);
    set_angle(1, m_oAngle, 1);
    goto_xy(0, 0);
}

void Scanner::SetJumpSpeed(double p_oJumpSpeedInBitsPerMs)
{
    set_jump_speed_ctrl(p_oJumpSpeedInBitsPerMs);
}

void Scanner::SetMarkSpeed(double p_oMarkSpeedInBitsPerMs)
{
    set_mark_speed_ctrl(p_oMarkSpeedInBitsPerMs);
}

void Scanner::GotoXY(long int p_oPosXinBits, long int p_oPosYinBits)
{
    goto_xy(p_oPosXinBits, p_oPosYinBits);
}

void Scanner::GetScannerError(uint32_t& p_rError, uint32_t& p_rLastError)
{
    p_rError = get_error();
    p_rLastError = get_last_error();
}

void Scanner::ResetScannerError(void)
{
    reset_error(0xFFFFFFFF); // reset the accumulated RTC error
}

uint32_t Scanner::GetHeadStatus(uint32_t p_oHead)
{
    return get_head_status(p_oHead);
}

int32_t Scanner::ReadControlValue(uint32_t p_oHead, uint32_t p_oAxis, uint32_t p_oCommand)
{
    control_command(p_oHead, p_oAxis, p_oCommand);
    return get_value(p_oAxis);
}

void Scanner::DriveWithOCTReferenceList(double p_oJumpSpeedInBitsPerMs, long int p_oPosXinBits, long int p_oPosYinBits, uint16_t p_oBinaryValue, uint16_t p_oBinaryMask)
{
    // code out of define_MiniSeamInit
    while (load_list(1,0) == 0);

    // code out of define_MiniSeamLine
    set_jump_speed(p_oJumpSpeedInBitsPerMs);
    write_io_port_mask_list(p_oBinaryValue, p_oBinaryMask);
    jump_abs(p_oPosXinBits, p_oPosYinBits);

    // code out of define_MiniSeamEnd
    set_end_of_list();

    // code out of start_Mark
    UINT oStatus = 0x01; // trigger reading of status
    UINT oPos = 0;
    while ((oStatus & 1) == 1)
    {
        get_status(&oStatus, &oPos);
    }
    execute_list(1);
}

void Scanner::DriveWithOCTReferenceControl(long int p_oPosXinBits, long int p_oPosYinBits, uint16_t p_oBinaryValue, uint16_t p_oBinaryMask)
{
    write_io_port_mask(p_oBinaryValue, p_oBinaryMask);
    goto_xy(p_oPosXinBits, p_oPosYinBits);
}

void Scanner::SetOCTReferenceControl(uint16_t p_oBinaryValue, uint16_t p_oBinaryMask)
{
    write_io_port_mask(p_oBinaryValue, p_oBinaryMask);
}

void Scanner::setScannerModel(ScannerModel newScannerModel)
{
    m_scannerModel = newScannerModel;
}

bool Scanner::correctionFileErrorMessage(unsigned int correctionFileErrorCode)
{
    ScannerLogger logger;
    return logger.correctionFileErrorMessage(static_cast<CorrectionFileErrorCode>(correctionFileErrorCode));
}

std::string Scanner::correctionFilePath(CorrectionFileType type) const
{
    auto wmBaseDir = std::string(getenv("WM_BASE_DIR"));
    switch (m_correctionFileType)
    {
        case(CorrectionFileType::lenseSpecific):
            return wmBaseDir + std::string("/calib/");
        case(CorrectionFileType::systemSpecific):
            return wmBaseDir + std::string("/config/calib/");
        default:
        __builtin_unreachable();
    }
}

std::string Scanner::correctionFileName(LensType lens) const
{
    switch (lens)
    {
    case LensType::F_Theta_340:
        return std::string("IntelliScanIII30_F_Theta_340");
    case LensType::F_Theta_460:
        return std::string("IntelliScanIII30_F_Theta_460");
    case LensType::F_Theta_255:
        return std::string("IntelliScanIII30_F_Theta_255");
    default:
        __builtin_unreachable();
    }
}

void Scanner::checkCustomCorrectionFileSpecified()
{
    if (m_customCorrectionFile.empty())
    {
        m_correctionFileType = CorrectionFileType::lenseSpecific;
        m_oCorrectionFile = correctionFileName(m_lensType);
        return;
    }

    m_correctionFileType = CorrectionFileType::systemSpecific;
    m_oCorrectionFile = m_customCorrectionFile;
}

void Scanner::applyScannerDefaultSettings() const
{
    if (m_scannerModel == ScannerModel::SmartMoveScanner)
    {
        set_matrix(1, 0, 1, 1, 0, 1);
    }

    set_offset(1, m_oXOffset, m_oYOffset, 1);
    set_angle(1, m_oAngle, 1);
    set_scanner_delays(m_oJumpDelay, m_oMarkDelay, m_oPolygonDelay);
}

int Scanner::loadCorrectionFile(const std::string& filename, int correctionFileNumber)
{
    return load_correction_file(filename.c_str(), correctionFileNumber, CORRECTION_FILE_DIMENSION_MODE);
}

bool Scanner::checkErrorCode(int errorCode)
{
    if (correctionFileErrorMessage(errorCode))
    {
        free_rtc6_dll();
        return true;
    }
    return false;
}

bool Scanner::loadCorrectionFiles()
{
    ScannerLogger logger;
    for (std::size_t i = 1; i <= static_cast<std::size_t> (CorrectionFileMode::HeightMeasurement); i++)
    {
        std::string filename;
        if (m_correctionFileType == CorrectionFileType::systemSpecific)
        {
            filename = appendCorrectionFileName(removeCorrectionFileSuffix(m_customCorrectionFile), static_cast<CorrectionFileMode> (i));
        }
        else
        {
            filename = appendCorrectionFileName(correctionFileName(m_lensType), static_cast<CorrectionFileMode> (i));
        }
        if (checkCorrectionFileExist(i, filename))
        {
            auto errorCode = loadCorrectionFile(filename, i);
            logger.logCorrectionFileNumber(i);
            auto currentCorrectionFileFailed = checkErrorCode(errorCode);
            if ((i == static_cast<std::size_t> (CorrectionFileMode::Welding)) && currentCorrectionFileFailed)
            {
                return true;
            }
        }
        if (!m_weldingCorrectionFileSelectable)
        {
            return true;
        }
    }
    return false;
}

std::string Scanner::correctionFileNameExtension(CorrectionFileMode mode)
{
    switch (mode)
    {
        case CorrectionFileMode::Welding:
            return {};
        case CorrectionFileMode::Pilot:
            return std::string("Pilot");
        case CorrectionFileMode::HeightMeasurement:
            return std::string("HeightMeasurement");
        default:
            __builtin_unreachable();
    }
}

std::string Scanner::correctionFileSuffix()
{
    return std::string(".ct5");
}

std::string Scanner::appendCorrectionFileName(const std::string& filename, CorrectionFileMode mode)
{
    return correctionFilePath(m_correctionFileType) + filename + correctionFileNameExtension(mode) + correctionFileSuffix();
}

std::string Scanner::removeCorrectionFileSuffix(const std::string& correctionFile)
{
    const auto& suffix = correctionFileSuffix();
    if (correctionFile.substr(correctionFile.size() - suffix.size()) != suffix)
    {
        return correctionFile;
    }

    const auto& correctionFilenameWithoutSuffix = correctionFile.substr(0, correctionFile.size() - suffix.size());

    return correctionFilenameWithoutSuffix;
}

void Scanner::setCorrectionFileSelectable(std::size_t i, bool selectable)
{
    auto mode = static_cast<CorrectionFileMode> (i);
    switch (mode)
    {
        case CorrectionFileMode::Welding:
            m_weldingCorrectionFileSelectable = selectable;
            return;
        case CorrectionFileMode::Pilot:
            m_pilotCorrectionFileSelectable = selectable;
            return;
        case CorrectionFileMode::HeightMeasurement:
            m_measurementCorrectionFileSelectable = selectable;
            return;
        default:
            __builtin_unreachable();
    }
}

bool Scanner::checkCorrectionFileModeIsSelectable(CorrectionFileMode mode)
{
    switch (mode)
    {
        case CorrectionFileMode::Welding:
            return m_weldingCorrectionFileSelectable;
        case CorrectionFileMode::Pilot:
            return m_pilotCorrectionFileSelectable;
        case CorrectionFileMode::HeightMeasurement:
            return m_measurementCorrectionFileSelectable;
        default:
            __builtin_unreachable();
    }
}

bool Scanner::checkCorrectionFileExist(std::size_t i, const std::string& filename)
{
    auto exists = checkFileExists(filename);
    if (!exists)
    {
        ScannerLogger logger;
        logger.logCorrectionFileIsMissing(i);
        logger.logCorrectionFileName(filename);
    }
    setCorrectionFileSelectable(i, exists);

    return exists;
}

bool Scanner::checkFileExists(const std::string& filename)
{
    std::ifstream file;
    file.open(filename);
    if (file)
    {
        file.close();
        return true;
    }

    file.close();
    return false;
}
}
