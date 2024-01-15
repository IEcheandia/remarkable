#pragma once

#include "common/systemConfiguration.h"
#include "common/definesScanlab.h"

using precitec::CorrectionFileErrorCode;
using precitec::interface::CorrectionFileMode;

namespace precitec
{
namespace hardware
{
namespace logger
{

/**
 * The class is used to sepparate the log messages from backend and functional parts.
 **/
class ScannerLogger
{
public:
    ScannerLogger();
    ~ScannerLogger();

    bool correctionFileErrorMessage(CorrectionFileErrorCode errorCode);
    void logCorrectionFileModeSelectionFailed(CorrectionFileMode mode);
    void logCorrectionFileNumber(std::size_t correctionFileNumber);
    void logCorrectionFileIsMissing(std::size_t correctionFileNumber);
    void logCorrectionFileName(const std::string& filename);
    void logCorrectionFileChangeToDefault();
    void logCorrectionFileMode(CorrectionFileMode mode);

private:
    void logCalibrationFactor(double calibrationFactor);
    std::string modeAsString(CorrectionFileMode mode);
};

}
}
}
