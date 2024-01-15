#include <calibration/chessboardWeldPointGenerator.h>
#include <math.h>
#include <common/systemConfiguration.h>
#include <geo/point.h>
#include <module/moduleLogger.h>

namespace precitec
{
namespace calibration_algorithm
{
ChessboardWeldPointGenerator::ChessboardWeldPointGenerator(double xMinScanfieldRange, double yMinScanfieldRange, double xMaxScanfieldRange, double yMaxScanfieldRange):
    m_lensType{interface::LensType(interface::SystemConfiguration::instance().getInt("ScanlabScanner_Lens_Type", 1))}
    , m_xMinScanfieldRange{xMinScanfieldRange}
    , m_yMinScanfieldRange{yMinScanfieldRange}
    , m_xMaxScanfieldRange{xMaxScanfieldRange}
    , m_yMaxScanfieldRange{yMaxScanfieldRange}
{
}


std::vector<geo2d::DPoint> ChessboardWeldPointGenerator::getWeldingPoints() const
{
    return createWeldingPoints();
}


geo2d::Size ChessboardWeldPointGenerator::getScanFieldSize() const
{
    switch (m_lensType)
    {
    case interface::LensType::F_Theta_340:
        // 220mm x 104mm
        return geo2d::Size{220, 104};
    case interface::LensType::F_Theta_460:
        // 380mm x 290mm
        return geo2d::Size{380, 290};
    case interface::LensType::F_Theta_255:
        // 170mm x 100mm
        return geo2d::Size{170, 100};
    default:
        __builtin_unreachable();
    }
}


bool ChessboardWeldPointGenerator::isPointWithinEllipse(const geo2d::DPoint& point, const geo2d::DPoint& center, const double& xLength, const double& yLength)
{
    return 1 >= (std::pow(point.x - center.x, 2) / std::pow(xLength, 2)) + (std::pow(point.y - center.y, 2) / std::pow(yLength, 2));
}

bool ChessboardWeldPointGenerator::isPointWithinROI(const geo2d::DPoint& point) const
{
    const auto xMax = std::max(std::abs(m_xMaxScanfieldRange), std::abs(m_xMinScanfieldRange));
    const auto xMin = -xMax;
    const auto yMax = std::max(std::abs(m_yMaxScanfieldRange), std::abs(m_yMinScanfieldRange));
    const auto yMin = -yMax;
    return point.x >= xMin && point.x <= xMax && point.y >= yMin && point.y <= yMax;

}

std::vector<geo2d::DPoint> ChessboardWeldPointGenerator::createWeldingPoints() const
{
    const auto scanFieldSize = getScanFieldSize();
    const auto rX = scanFieldSize.width / 2;
    const auto rY = scanFieldSize.height / 2;
    const auto midX = rX - (rX % 10);
    const auto midY = rY - (rY % 10);
    std::vector<precitec::geo2d::DPoint> points;

    auto index = 0;
    for (int y = midY; y >= -midY; y-=10)
    {
        for (int x = -midX; x <= midX; x+=10)
        {
            const auto xCoord = index%2 != 0 ? x * -1 : x;
            const auto pointWithinROI = isPointWithinROI(precitec::geo2d::DPoint{static_cast<double>(xCoord), static_cast<double>(y)});
            if (pointWithinROI)
            {
                points.emplace_back(precitec::geo2d::DPoint{static_cast<double>(xCoord), static_cast<double>(y)});
            }
        }
        ++index;
    }
    wmLog(eDebug, "generates %d points\n", points.size());
    return points;
}

}
}
