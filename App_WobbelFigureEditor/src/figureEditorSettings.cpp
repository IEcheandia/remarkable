#include "figureEditorSettings.h"

#include <cmath>

namespace precitec
{

using interface::SystemConfiguration;
using interface::ScannerGeneralMode;
namespace scanmaster
{

namespace components
{

namespace wobbleFigureEditor
{

const int DEFAULT_SCALE(1000);
const int MAX_SCALE(1000);
const int MIN_SCALE(10);
const int SCALE_FACTOR(10);

FigureEditorSettings::FigureEditorSettings()
    : QObject()
    , m_dualChannelLaser(SystemConfiguration::instance().getBool("LaserControlTwoChannel", false))
    , m_digitalLaserPower(SystemConfiguration::instance().getBool("SCANMASTER_ThreeStepInterface", false))
    , m_scanMasterMode(static_cast<ScannerGeneralMode> (SystemConfiguration::instance().getInt("ScannerGeneralMode", static_cast<int> (ScannerGeneralMode::eScanMasterMode))) == ScannerGeneralMode::eScanMasterMode ? true : false)
    , m_scannerSpeed{1.0}
    , m_laserMaxPower(SystemConfiguration::instance().getInt("Maximum_Simulated_Laser_Power", 4000))
    , m_heatMap{HeatMap{0.0, QColor(Qt::blue)}, HeatMap{25.0, QColor(Qt::cyan)}, HeatMap{50.0, QColor(Qt::darkGreen)}, HeatMap{75.0, QColor(Qt::yellow)}, HeatMap{100.0, QColor(Qt::red)}}
    , m_lensType(LensType(SystemConfiguration::instance().getInt("ScanlabScanner_Lens_Type", 1)))
    , m_scale{DEFAULT_SCALE}
    , m_fileType{FileType::Seam}
{ }

FigureEditorSettings::~FigureEditorSettings() = default;

FigureEditorSettings * FigureEditorSettings::instance()
{
    static FigureEditorSettings s_instance;
    return &s_instance;
}

void FigureEditorSettings::setScannerSpeed(double newSpeed)
{
    if (qFuzzyCompare(m_scannerSpeed, newSpeed))
    {
        return;
    }

    m_scannerSpeed = newSpeed;
    emit scannerSpeedChanged();
}

void FigureEditorSettings::setLaserMaxPower(double newMaximumPower)
{
    if (qFuzzyCompare(m_laserMaxPower, newMaximumPower))
    {
        return;
    }

    m_laserMaxPower = newMaximumPower;
    emit laserMaxPowerChanged();
}

void FigureEditorSettings::setScale(int newScale)
{
    if (m_scale == newScale)
    {
        return;
    }

    m_scale = newScale;
    emit scaleChanged();
}

void FigureEditorSettings::setFileType(FileType type)
{
    if (m_fileType == type)
    {
        return;
    }

    m_fileType = type;
    emit fileTypeChanged();
}

double FigureEditorSettings::thresholdFromIndex(int index) const
{
    if (static_cast<std::size_t> (index) >= m_heatMap.size())
    {
        return {};
    }

    return m_heatMap.at(index).threshold;
}

QColor FigureEditorSettings::colorFromValue ( double value ) const
{
    if (m_heatMap.empty())
    {
        return {};
    }

    auto it = std::find_if(m_heatMap.begin(), m_heatMap.end(), [value] (const auto& heatMap) {return value <= heatMap.threshold; });

    if (it == m_heatMap.begin())
    {
        return m_heatMap.front().color;
    }
    if (it == m_heatMap.end())
    {
        return m_heatMap.back().color;
    }

    const auto& color1 = *std::prev(it);
    const auto& color2 = *it;

    const auto& normalizedValue = (value - color1.threshold) / (color2.threshold - color1.threshold);

    const auto& red =  (1.0 - normalizedValue) * color1.color.red() + normalizedValue * color2.color.red();
    const auto& green =  (1.0 - normalizedValue) * color1.color.green() + normalizedValue * color2.color.green();
    const auto& blue =  (1.0 - normalizedValue) * color1.color.blue() + normalizedValue * color2.color.blue();

    return {static_cast<int> (std::round(red)), static_cast<int> (std::round(green)), static_cast<int> (std::round(blue))};
}

void FigureEditorSettings::increaseScaleByScaleFactor()
{
    if (scale() * SCALE_FACTOR > MAX_SCALE)
    {
        return;
    }
    setScale(scale() * SCALE_FACTOR);
}

void FigureEditorSettings::decreaseScaleByScaleFactor()
{
    if (scale() / SCALE_FACTOR < MIN_SCALE)
    {
        return;
    }
    setScale(scale() / SCALE_FACTOR);
}

}
}
}
}
