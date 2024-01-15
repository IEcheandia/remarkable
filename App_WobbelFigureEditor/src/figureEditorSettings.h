#pragma once

#include <QObject>
#include <QColor>
#include "fileType.h"
#include "common/systemConfiguration.h"

using precitec::scantracker::components::wobbleFigureEditor::FileType;
using precitec::interface::LensType;

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

/**
* Singleton class providing figure editor configuration values.
**/
class FigureEditorSettings : public QObject
{
    Q_OBJECT
    /**
     * Option to enable dual channel laser control, dual channel laser control means that there is a core and ring laser power
     **/
    Q_PROPERTY(bool dualChannelLaser READ dualChannelLaser CONSTANT)
    /**
     * Option to enable if laser power is analog or digital for controlling laser with analog voltage or with bits
     **/
    Q_PROPERTY(bool digitalLaserPower READ digitalLaserPower CONSTANT)
    /**
     * Option to enable scanmaster mode which hides the basic wobble figures in figure editor
     **/
    Q_PROPERTY(bool scanMasterMode READ scanMasterMode CONSTANT)
    /**
     * Global scanner speed for the whole figure, doesn't work if speed is configured on the figure itself
     **/
    Q_PROPERTY(double scannerSpeed READ scannerSpeed WRITE setScannerSpeed NOTIFY scannerSpeedChanged)
    /**
     * Value to specify the maximum laser power of the laser, is used by power graph
     **/
    Q_PROPERTY(unsigned int laserMaxPower READ laserMaxPower WRITE setLaserMaxPower NOTIFY laserMaxPowerChanged)
    /**
     * Factor to calculate pixels (figure editor) from mm and mm from pixels.
     **/
    Q_PROPERTY(int scale READ scale WRITE setScale NOTIFY scaleChanged)
    /**
     * Type of which file (seam, wobble or overlay) is loaded or selected at the moment.
     **/
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::FileType fileType READ fileType WRITE setFileType NOTIFY fileTypeChanged)

public:
    ~FigureEditorSettings() override;

    static FigureEditorSettings* instance();

    struct HeatMap
    {
        double threshold;
        QColor color;
    };

    bool dualChannelLaser() const
    {
        return m_dualChannelLaser;
    }

    bool digitalLaserPower() const
    {
        return m_digitalLaserPower;
    }

    bool scanMasterMode()  const
    {
        return m_scanMasterMode;
    }

    double scannerSpeed() const
    {
        return m_scannerSpeed;
    }
    void setScannerSpeed(double newSpeed);

    unsigned int laserMaxPower() const
    {
        return m_laserMaxPower;
    }
    void setLaserMaxPower(double newMaximumPower);

    std::vector<HeatMap> heatMap() const
    {
        return m_heatMap;
    }

    LensType lensType() const
    {
        return m_lensType;
    }

    int scale() const
    {
        return m_scale;
    }
    void setScale(int newScale);

    FileType fileType() const
    {
        return m_fileType;
    }
    void setFileType(FileType type);

    Q_INVOKABLE double thresholdFromIndex(int index) const;
    Q_INVOKABLE QColor colorFromValue(double value) const;

    Q_INVOKABLE void increaseScaleByScaleFactor();
    Q_INVOKABLE void decreaseScaleByScaleFactor();

Q_SIGNALS:
    void scannerSpeedChanged();
    void laserMaxPowerChanged();
    void scaleChanged();
    void fileTypeChanged();

private:
    explicit FigureEditorSettings();

    bool m_dualChannelLaser;
    bool m_digitalLaserPower;
    bool m_scanMasterMode;
    double m_scannerSpeed;
    unsigned int m_laserMaxPower;
    std::vector<HeatMap> m_heatMap;
    LensType m_lensType;
    int m_scale;
    FileType m_fileType;
};

}
}
}
}
