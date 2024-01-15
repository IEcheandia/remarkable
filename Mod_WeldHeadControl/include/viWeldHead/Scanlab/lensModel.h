#pragma once

#include <string>
#include "common/systemConfiguration.h"
#include "lensData.h"

using precitec::interface::LensType;
using precitec::interface::ScannerModel;
using precitec::interface::CorrectionFileMode;

class LensModelTest;

namespace precitec
{
namespace hardware
{

/**
 * A class which is used to select the correct lens type.
 * After selecting the correct lens type the important data can be returned.
 **/
class LensModel
{
public:
    LensModel();
    ~LensModel();

    LensType type() const
    {
        return m_type;
    }
    void setType(LensType newLens);
    ScannerModel scannerController() const
    {
        return m_scannerController;
    }
    void setScannerController(ScannerModel newScannerController);

    LensData currentLensInformation() const;

private:
    LensData lensData() const;
    std::string correctionFile(CorrectionFileMode mode) const;
    std::string correctionFilePrefix() const;
    std::string correctionFileLens() const;
    std::string correctionFileLensScanlab() const;
    std::string correctionFileLensSmartMove() const;
    std::string correctionFileMode(CorrectionFileMode mode) const;
    std::string correctionFileSuffix() const;

    LensType m_type{LensType::F_Theta_340};
    ScannerModel m_scannerController{ScannerModel::ScanlabScanner};

    friend LensModelTest;
};

}
}
