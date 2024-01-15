#include "viWeldHead/Scanlab/lensModel.h"

namespace precitec
{
namespace hardware
{

LensModel::LensModel()
{
}

LensModel::~LensModel() = default;

void LensModel::setType(precitec::interface::LensType newLens)
{
    m_type = newLens;
}

void LensModel::setScannerController(precitec::interface::ScannerModel newScannerController)
{
    m_scannerController = newScannerController;
}

LensData LensModel::currentLensInformation() const
{
    return lensData();
}

LensData LensModel::lensData() const
{
    switch (m_type)
    {
        case LensType::F_Theta_340:
        {
            LensData lens340;
            lens340.type = precitec::interface::LensType::F_Theta_340;
            lens340.focalLength = 340;
            lens340.scanFieldSize = std::make_pair(220, 104);
            lens340.scanFieldSquare = 52;
            lens340.calibrationFile = correctionFile(CorrectionFileMode::Welding);
            lens340.calibrationFilePilot = correctionFile(CorrectionFileMode::Pilot);
            lens340.calibrationFileZMeasurement = correctionFile(CorrectionFileMode::HeightMeasurement);
            return lens340;
        }
        case LensType::F_Theta_460:
        {
            LensData lens460;
            lens460.type = precitec::interface::LensType::F_Theta_460;
            lens460.focalLength = 460;
            lens460.scanFieldSize = std::make_pair(380, 290);
            lens460.scanFieldSquare = 145;
            lens460.calibrationFile = correctionFile(CorrectionFileMode::Welding);
            lens460.calibrationFilePilot = correctionFile(CorrectionFileMode::Pilot);
            lens460.calibrationFileZMeasurement = correctionFile(CorrectionFileMode::HeightMeasurement);
            return lens460;
        }
        case LensType::F_Theta_255:
        {
            LensData lens255;
            lens255.type = precitec::interface::LensType::F_Theta_255;
            lens255.focalLength = 255;
            lens255.scanFieldSize = std::make_pair(170, 100);
            lens255.scanFieldSquare = 50;
            lens255.calibrationFile = correctionFile(CorrectionFileMode::Welding);
            lens255.calibrationFilePilot = correctionFile(CorrectionFileMode::Pilot);
            lens255.calibrationFileZMeasurement = correctionFile(CorrectionFileMode::HeightMeasurement);
            return lens255;
        }
        default:
            __builtin_unreachable();
    }
}

std::string LensModel::correctionFile(CorrectionFileMode mode) const
{
    return correctionFilePrefix() + correctionFileLens() + correctionFileMode(mode) + correctionFileSuffix();
}

std::string LensModel::correctionFilePrefix() const
{
    switch (m_scannerController)
    {
        case ScannerModel::ScanlabScanner:
        {
            return std::string("IntelliScanIII30");
        }
        case ScannerModel::SmartMoveScanner:
        {
            return std::string("ME");
        }
        default:
            __builtin_unreachable();
    }
}

std::string LensModel::correctionFileLens() const
{
    switch (m_scannerController)
    {
        case ScannerModel::ScanlabScanner:
        {
            return correctionFileLensScanlab();
        }
        case ScannerModel::SmartMoveScanner:
        {
            return correctionFileLensSmartMove();
        }
        default:
            __builtin_unreachable();
    }
}

std::string LensModel::correctionFileLensScanlab() const
{
    switch (m_type)
    {
        case LensType::F_Theta_340:
        {
            return {"_F_Theta_340"};
        }
        case LensType::F_Theta_460:
        {
            return {"_F_Theta_460"};
        }
        case LensType::F_Theta_255:
        {
            return {"_F_Theta_255"};
        }
        default:
            __builtin_unreachable();
    }
}

std::string LensModel::correctionFileLensSmartMove() const
{
    switch (m_type)
    {
        case LensType::F_Theta_340:
        {
            return {"_F340"};
        }
        case LensType::F_Theta_460:
        {
            return {"_F460"};
        }
        case LensType::F_Theta_255:
        {
            return {"_F255"};
        }
        default:
            __builtin_unreachable();
    }
}

std::string LensModel::correctionFileMode(CorrectionFileMode mode) const
{
    if (m_scannerController == ScannerModel::SmartMoveScanner)
    {
        return {};
    }

    switch (mode)
    {
        case CorrectionFileMode::Welding:
        {
            return {};
        }
        case CorrectionFileMode::Pilot:
        {
            return {"Pilot"};
        }
        case CorrectionFileMode::HeightMeasurement:
        {
            return {"HeightMeasurement"};
        }
        default:
            __builtin_unreachable();
    }
}

std::string LensModel::correctionFileSuffix() const
{
    switch (m_scannerController)
    {
        case ScannerModel::ScanlabScanner:
        {
            return std::string(".ct5");
        }
        case ScannerModel::SmartMoveScanner:
        {
            return std::string(".sbd");
        }
        default:
            __builtin_unreachable();
    }
}

}
}
