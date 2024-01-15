#include "viWeldHead/Scanlab/scannerLogger.h"

#include "module/moduleLogger.h"

namespace precitec
{
namespace hardware
{
namespace logger
{

ScannerLogger::ScannerLogger()
{
}

ScannerLogger::~ScannerLogger() = default;

bool ScannerLogger::correctionFileErrorMessage(CorrectionFileErrorCode errorCode)
{
    switch (errorCode)
    {
        case CorrectionFileErrorCode::Success:
            wmLogTr(eDebug, "QnxMsg.VI.CorFileSuccesLoad", "Correction file loaded successfully\n");
            return false;
        case CorrectionFileErrorCode::FileCorrupt:
            wmLogTr(eError, "QnxMsg.VI.CorFileCorrupted", "Correction file is corrupted\n");
            break;
        case CorrectionFileErrorCode::Memory:
            wmLogTr(eError, "QnxMsg.VI.CorFileMemory", "Memory error while saving correction file\n");
            break;
        case CorrectionFileErrorCode::FileNotFound:
            wmLogTr(eError, "QnxMsg.VI.CorFileNotFound", "Correction file not found\n");
            break;
        case CorrectionFileErrorCode::DSPMemory:
            wmLogTr(eError, "QnxMsg.VI.CorFileDSPMemory", "DSP memory error\n");
            break;
        case CorrectionFileErrorCode::PCIDownload:
            wmLogTr(eError, "QnxMsg.VI.CorFilePCIDownload", "PCI download error\n");
            break;
        case CorrectionFileErrorCode::RTC6Driver:
            wmLogTr(eError, "QnxMsg.VI.CorFileRTC6Driver", "RTC6 card driver not found\n");
            break;
        case CorrectionFileErrorCode::CorrectionFileNumber:
            wmLogTr(eError, "QnxMsg.VI.CorFileNumber", "Correction file number is out of bounds\n");
            break;
        case CorrectionFileErrorCode::Access:
            wmLogTr(eError, "QnxMsg.VI.CorFileAccess", "Access denied. Check firmware files\n");
            break;
        case CorrectionFileErrorCode::Option3DUnlocked:
            wmLogTr(eError, "QnxMsg.VI.CorFile3DOption", "3D correction file choosen but 3D option is unlocked\n");
            break;
        case CorrectionFileErrorCode::Busy:
            wmLogTr(eError, "QnxMsg.VI.CorFileBusy", "RTC6 card is busy which means already applying a list or a jump\n");
            break;
        case CorrectionFileErrorCode::PCIUpload:
            wmLogTr(eError, "QnxMsg.VI.CorFilePCIUpload", "PCI upload error\n");
            break;
        case CorrectionFileErrorCode::Verify:
            wmLogTr(eError, "QnxMsg.VI.CorFileVerify", "Download verification failed\n");
            break;
        default:
            __builtin_unreachable();
    }
    return true;
}

void ScannerLogger::logCorrectionFileModeSelectionFailed(CorrectionFileMode mode)
{
    switch (mode)
    {
        case CorrectionFileMode::Welding:
            wmFatal(eAxis, "QnxMsg.VI.CorFileWeldCorrupt", "No welding correction file available\n");
            return;
        case CorrectionFileMode::Pilot:
            wmLogTr(eWarning, "QnxMsg.VI.CorFileSelectFailed", "Select cor file mode failed: %s\n", "Pilot");
            return;
        case CorrectionFileMode::HeightMeasurement:
            wmLogTr(eWarning, "QnxMsg.VI.CorFileSelectFailed", "Select cor file mode failed: %s\n", "HeightMeasurement");
            return;
        default:
            __builtin_unreachable();
    }
}

void ScannerLogger::logCorrectionFileNumber(std::size_t correctionFileNumber)
{
    wmLogTr(eDebug, "QnxMsg.VI.CorFileSelected", "Current cor file is %s\n", modeAsString(static_cast<CorrectionFileMode>(correctionFileNumber)));
}

void ScannerLogger::logCorrectionFileIsMissing(std::size_t correctionFileNumber)
{
    wmLogTr(eWarning, "QnxMsg.VI.CorFileSelected", "Missing cor file %s\n", modeAsString(static_cast<CorrectionFileMode>(correctionFileNumber)));
}

void ScannerLogger::logCorrectionFileName(const std::string& filename)
{
    wmLogTr(eWarning, "QnxMsg.VI.CorFileSelected", "Name: %s\n", filename);
}

void ScannerLogger::logCorrectionFileChangeToDefault()
{
    wmLogTr(eError, "QnxMsg.VI.CorFileDefault", "Set correction file mode to welding\n");
}

void ScannerLogger::logCorrectionFileMode(CorrectionFileMode mode)
{
    wmLogTr(eDebug, "QnxMsg.VI.CorFileMode", "New correction mode: %s\n", modeAsString(mode));
}

void ScannerLogger::logCalibrationFactor(double calibrationFactor)
{
    wmLogTr(eDebug, "QnxMsg.VI.CalibrationFactor", "New calibration factor: %f\n", calibrationFactor);
}

std::string ScannerLogger::modeAsString(CorrectionFileMode mode)
{
    switch (mode)
    {
        case CorrectionFileMode::Welding:
            return std::string("Welding");
        case CorrectionFileMode::Pilot:
            return std::string("Pilot");
        case CorrectionFileMode::HeightMeasurement:
            return std::string("HeightMeasurement");
        default:
            __builtin_unreachable();
    }
}

}
}
}
