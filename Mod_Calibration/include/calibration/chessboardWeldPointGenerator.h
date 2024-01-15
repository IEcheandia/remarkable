#pragma once

#include "geo/size.h"
#include "geo/point.h"
#include "common/systemConfiguration.h"

namespace precitec
{
namespace calibration_algorithm
{

class ChessboardWeldPointGenerator
{

public:

    ChessboardWeldPointGenerator(double xMinScanfieldRange, double yMinScanfieldRange, double xMaxScanfieldRange, double yMaxScanfieldRange);

    std::vector<geo2d::DPoint> getWeldingPoints() const;

private:

    std::vector<geo2d::DPoint> createWeldingPoints() const;
    geo2d::Size getScanFieldSize() const;
    static bool isPointWithinEllipse(const geo2d::DPoint& point, const geo2d::DPoint& center, const double& xLength, const double& yLength);
    bool isPointWithinROI(const geo2d::DPoint& point) const;

    interface::LensType m_lensType;
    geo2d::Size m_scanFieldSize;
    double m_xMinScanfieldRange;
    double m_yMinScanfieldRange;
    double m_xMaxScanfieldRange;
    double m_yMaxScanfieldRange;
};

}
}
