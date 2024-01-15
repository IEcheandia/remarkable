#pragma once

#include <string>
#include <cstdint>

#include "common/systemConfiguration.h"

using precitec::interface::ScannerModel;
using precitec::interface::LensType;
using precitec::interface::CorrectionFileMode;

class ScannerTest;

namespace RTC6
{
class Scanner
{
    public:
        enum class CorrectionFileType
        {
            systemSpecific = 0,
            lenseSpecific
        };
        Scanner();
        Scanner(long int p_oXOffset, long int p_oYOffset, double p_oAngle, long int p_oJumpDelay, long int p_oMarkDelay, long int p_oPolygonDelay);
        bool init();
        double selectCorrectionFile(CorrectionFileMode mode);
        void setCustomCorrectionFile(const std::string& customCorrectionFile);
        void setLensType(LensType lens);

        void set_CorrectionFile(const std::string &p_oCorrectionFile);
        void set_CorrectionFileType(const CorrectionFileType& type);
        void set_ScannerOffset(long int p_oXOffset, long int p_oYOffset, double p_oAngle);
        void set_ScannerDelays(long int p_oJumpDelay, long int p_oMarkDelay, long int p_oPolygonDelay);
        std::string correctionFile() const
        {
            return m_oCorrectionFile;
        }
        void get_ScannerOffset(long int& p_rXOffset, long int& p_rYOffset, double& p_rAngle) const;
        void get_ScannerDelays(long int& p_rJumpDelay, long int& p_rMarkDelay, long int& p_rPolygonDelay);
        double get_CalibValueBitsPerMM(void) const;
        //int init();
        void jump_ScannerOffset();
        void SetJumpSpeed(double p_oJumpSpeedInBitsPerMs);
        void SetMarkSpeed(double p_oMarkSpeedInBitsPerMs);
        void GotoXY(long int p_oPosXinBits, long int p_oPosYinBits);
        void GetScannerError(uint32_t& p_rError, uint32_t& p_rLastError);
        void ResetScannerError(void);
        uint32_t GetHeadStatus(uint32_t p_oHead);
        int32_t ReadControlValue(uint32_t p_oHead, uint32_t p_oAxis, uint32_t p_oCommand);
        void DriveWithOCTReferenceList(double p_oJumpSpeedInBitsPerMs, long int p_oPosXinBits, long int p_oPosYinBits, uint16_t p_oBinaryValue, uint16_t p_oBinaryMask);
        void DriveWithOCTReferenceControl(long int p_oPosXinBits, long int p_oPosYinBits, uint16_t p_oBinaryValue, uint16_t p_oBinaryMask);
        void SetOCTReferenceControl(uint16_t p_oBinaryValue, uint16_t p_oBinaryMask);

        ScannerModel scannerModel()
        {
            return m_scannerModel;
        }
        void setScannerModel(ScannerModel newScannerModel);

    private:
        bool correctionFileErrorMessage(unsigned int correctionFileErrorCode);
        std::string correctionFilePath(CorrectionFileType type) const;
        std::string correctionFileName(LensType lens) const;
        void checkCustomCorrectionFileSpecified();
        void applyScannerDefaultSettings() const;
        int loadCorrectionFile(const std::string& filename, int correctionFileNumber);
        bool checkErrorCode(int errorCode);
        bool loadCorrectionFiles();
        std::string correctionFileNameExtension(CorrectionFileMode mode);
        std::string correctionFileSuffix();
        std::string appendCorrectionFileName(const std::string& filename, CorrectionFileMode mode);
        std::string removeCorrectionFileSuffix(const std::string& correctionFile);
        void setCorrectionFileSelectable(std::size_t i, bool selectable);
        bool checkCorrectionFileModeIsSelectable(CorrectionFileMode mode);
        bool checkCorrectionFileExist(std::size_t i, const std::string& filename);
        bool checkFileExists(const std::string& filename);

        bool m_weldingCorrectionFileSelectable{false};
        bool m_pilotCorrectionFileSelectable{false};
        bool m_measurementCorrectionFileSelectable{false};
        std::string m_customCorrectionFile;
        std::string m_oCorrectionFile;
        std::string m_pilotCorrectionFile;
        std::string m_measurementCorrectionFile;
        CorrectionFileType m_correctionFileType;
        ScannerModel m_scannerModel = ScannerModel::ScanlabScanner;
        LensType m_lensType{LensType::F_Theta_340};
        double m_oCalibValueBitsPerMM;
        long int m_oXOffset;
        long int m_oYOffset;
        double m_oAngle;
        long int m_oJumpDelay;
        long int m_oMarkDelay;
        long int m_oPolygonDelay;

        friend ScannerTest;
};
}
