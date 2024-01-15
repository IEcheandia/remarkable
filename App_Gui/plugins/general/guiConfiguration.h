#pragma once

#include <QObject>
#include <QLocale>
#include <QUuid>

class QTranslator;

namespace precitec
{
namespace gui
{

class FileSystemNameValidator;

/**
 * Singleton class providing gui configuration values.
 **/
class GuiConfiguration : public QObject
{
    Q_OBJECT
    /**
     * Path to the gui configuration file
     **/
    Q_PROPERTY(QString configFilePath READ configFilePath NOTIFY configFilePathChanged)
    /**
     * Feature: seam series as structured elements on Product structure. If @c true the product contains seam series, if @c false (default)
     * the product provides seams directly without exposing the underlying seam series.
     **/
    Q_PROPERTY(bool seamSeriesOnProductStructure READ seamSeriesOnProductStructure WRITE setSeamSeriesOnProductStructure NOTIFY seamSeriesOnProductStructureChanged)
    /**
     * Feature: seam intervals as structured elements on Product structure. If @c true the seam contains seam intervals, if @c false (default)
     * the seam intervals are only configured indirectly through the seam.
     **/
    Q_PROPERTY(bool seamIntervalsOnProductStructure READ seamIntervalsOnProductStructure WRITE setSeamIntervalsOnProductStructure NOTIFY seamIntervalsOnProductStructureChanged)
    /**
     * Feature: configure the voltages black level offset on Hardware ROI page.
     * Default is @c false
     **/
    Q_PROPERTY(bool configureBlackLevelOffsetVoltagesOnCamera READ configureBlackLevelOffsetVoltagesOnCamera WRITE setConfigureBlackLevelOffsetVoltagesOnCamera NOTIFY configureBlackLevelOffsetVoltagesOnCameraChanged)
    /**
     * Feature: configure lin log on Hardware ROI page
     * Default is @c false
     **/
    Q_PROPERTY(bool configureLinLogOnCamera READ configureLinLogOnCamera WRITE setConfigureLinLogOnCamera NOTIFY configureLinLogOnCameraChanged)
    /**
     * Feature: configure thickness of seam.
     * Default is @c false
     **/
    Q_PROPERTY(bool configureThicknessOnSeam READ configureThicknessOnSeam WRITE setConfigureThicknessOnSeam NOTIFY configureThicknessOnSeamChanged)
    /**
     * Feature: configure moving direction of seam.
     * Default is @c false
     **/
    Q_PROPERTY(bool configureMovingDirectionOnSeam READ configureMovingDirectionOnSeam WRITE setConfigureMovingDirectionOnSeam NOTIFY configureMovingDirectionOnSeamChanged)
    /**
     * Feature: LED calibration in Wizard
     * Default is @c false
     **/
    Q_PROPERTY(bool ledCalibration READ ledCalibration WRITE setLedCalibration NOTIFY ledCalibrationChanged)

    /**
     * Feature: edit filter parameters of user level operator directly in SeamConfiguration
     * Default is @c false
     **/
    Q_PROPERTY(bool quickEditFilterParametersOnSeam READ quickEditFilterParametersOnSeam WRITE setQuickEditFilterParametersOnSeam NOTIFY quickEditFilterParametersOnSeamChanged)

    /**
     * Feature: add quality fault Q to X and quality fault category 2 templates to ErrorSettings
     * Default is @c false
     **/
    Q_PROPERTY(bool qualityFaultCategory2 READ qualityFaultCategory2 WRITE setQualityFaultCategory2 NOTIFY qualityFaultCategory2Changed)

    /**
     * Feature: decide if the vertical scale of the plotter is determined by the values in the Result and Error Settings
     * Default is @c false
     **/
    Q_PROPERTY(bool scalePlotterFromSettings READ scalePlotterFromSettings WRITE setScalePlotterFromSettings NOTIFY scalePlotterFromSettingsChanged)

    /**
     * Feature: decide if the graph editor appears in the tab bar, renamed to avoid easily access to the graph editor.
     * Default is @c false
     **/
    Q_PROPERTY(bool formatHardDisk READ formatHardDisk WRITE setFormatHardDisk NOTIFY formatHardDiskChanged)

    /**
     * The maximum number of screenshots to keep.
     **/
    Q_PROPERTY(uint maximumNumberOfScreenshots READ maximumNumberOfScreenshots WRITE setMaximumNumberOfScreenshots NOTIFY maximumNumberOfScreenshotsChanged)
    Q_PROPERTY(uint maximumNumberOfScreenshotsMin READ maximumNumberOfScreenshotsMin CONSTANT)
    Q_PROPERTY(uint maximumNumberOfScreenshotsMax READ maximumNumberOfScreenshotsMax CONSTANT)
    Q_PROPERTY(uint maximumNumberOfScreenshotsDefault READ defaultMaximumNumberOfScreenshots CONSTANT)

    /**
     * The maximum number of seams to display on the Overview page.
     **/
    Q_PROPERTY(uint maximumNumberOfSeamsOnOverview READ maximumNumberOfSeamsOnOverview WRITE setMaximumNumberOfSeamsOnOverview NOTIFY maximumNumberOfSeamsOnOverviewChanged)
    Q_PROPERTY(uint maximumNumberOfSeamsOnOverviewMin READ maximumNumberOfSeamsOnOverviewMin CONSTANT)
    Q_PROPERTY(uint maximumNumberOfSeamsOnOverviewMax READ maximumNumberOfSeamsOnOverviewMax CONSTANT)
    Q_PROPERTY(uint maximumNumberOfSeamsOnOverviewDefault READ defaultMaximumNumberOfSeamsOnOverview CONSTANT)

    Q_PROPERTY(uint numberOfSeamsInPlotter READ numberOfSeamsInPlotter WRITE setNumberOfSeamsInPlotter NOTIFY numberOfSeamsInPlotterChanged)
    Q_PROPERTY(uint numberOfSeamsInPlotterMin READ numberOfSeamsInPlotterMin CONSTANT)
    Q_PROPERTY(uint numberOfSeamsInPlotterMax READ numberOfSeamsInPlotterMax CONSTANT)
    Q_PROPERTY(uint numberOfSeamsInPlotterDefault READ defaultNumberOfSeamsInPlotter CONSTANT)
    /**
     * The unique id of the station.
     **/
    Q_PROPERTY(QUuid stationId READ stationId CONSTANT)

    /**
     * The name of the station, initialized with station id.
     **/
    Q_PROPERTY(QString stationName READ stationName WRITE setStationName NOTIFY stationNameChanged)
    Q_PROPERTY(QString stationNameDefault READ defaultStationName CONSTANT)

    Q_PROPERTY(QString roleNameViewUser READ roleNameViewUser WRITE setRoleNameViewUser NOTIFY roleNameViewUserChanged)
    Q_PROPERTY(QString roleNameViewUserDefault READ defaultRoleNameViewUser CONSTANT)

    Q_PROPERTY(QStringList availableLanguages READ availableLanguages CONSTANT)
    Q_PROPERTY(QString language READ language NOTIFY languageChanged)
    Q_PROPERTY(QString uiLanguage READ uiLanguage NOTIFY languageChanged)

    Q_PROPERTY(QLocale locale READ locale NOTIFY localeChanged)

    /**
     * Feature: decide if signals should be colored by their signalQuality or not
     * Default is @c false
     **/
    Q_PROPERTY(bool colorSignalsByQuality READ colorSignalsByQuality WRITE setColorSignalsByQuality NOTIFY colorSignalsByQualityChanged)

    /**
     * Feature: decide if signals error boundaries should be displayed in plots
     * Default is @c true
     **/
    Q_PROPERTY(bool displayErrorBoundariesInPlotter READ displayErrorBoundariesInPlotter WRITE setDisplayErrorBoundariesInPlotter NOTIFY displayErrorBoundariesInPlotterChanged)

    /**
     * Whether virtual keyboard support is available for touch.
     **/
    Q_PROPERTY(bool virtualKeyboard READ virtualKeyboard WRITE setVirtualKeyboard NOTIFY virtualKeyboardChanged)

    /**
     * Whether remote desktop support (VNC) should be enabled on system startup.
     **/
    Q_PROPERTY(bool remoteDesktopOnStartup READ remoteDesktopOnStartup WRITE setRemoteDesktopOnStartup NOTIFY remoteDesktopOnStartupChanged)

    /**
     * Whether the gui is blocked during automatic mode.
     **/
    Q_PROPERTY(bool blockedAutomatic READ blockedAutomatic WRITE setBlockedAutomatic NOTIFY blockedAutomaticChanged)

    Q_PROPERTY(uint serialNumberFromExtendedProductInfo READ serialNumberFromExtendedProductInfo WRITE setSerialNumberFromExtendedProductInfo NOTIFY serialNumberFromExtendedProductInfoChanged)
    Q_PROPERTY(uint serialNumberFromExtendedProductInfoMin READ serialNumberFromExtendedProductInfoMin CONSTANT)
    Q_PROPERTY(uint serialNumberFromExtendedProductInfoMax READ serialNumberFromExtendedProductInfoMax CONSTANT)
    Q_PROPERTY(uint serialNumberFromExtendedProductInfoDefault READ serialNumberFromExtendedProductInfoDefault CONSTANT)

    Q_PROPERTY(uint partNumberFromExtendedProductInfo READ partNumberFromExtendedProductInfo WRITE setPartNumberFromExtendedProductInfo NOTIFY partNumberFromExtendedProductInfoChanged)
    Q_PROPERTY(uint partNumberFromExtendedProductInfoMin READ partNumberFromExtendedProductInfoMin CONSTANT)
    Q_PROPERTY(uint partNumberFromExtendedProductInfoMax READ partNumberFromExtendedProductInfoMax CONSTANT)
    Q_PROPERTY(uint partNumberFromExtendedProductInfoDefault READ partNumberFromExtendedProductInfoDefault CONSTANT)

    /**
     * Maximum time in s the Plotter shows the samples and results for live mode
     **/
    Q_PROPERTY(uint maxTimeLiveModePlotter READ maxTimeLiveModePlotter WRITE setMaxTimeLiveModePlotter NOTIFY maxTimeLiveModePlotterChanged)
    Q_PROPERTY(uint maxTimeLiveModePlotterMin READ maxTimeLiveModePlotterMin CONSTANT)
    Q_PROPERTY(uint maxTimeLiveModePlotterMax READ maxTimeLiveModePlotterMax CONSTANT)
    Q_PROPERTY(uint maxTimeLiveModePlotterDefault READ maxTimeLiveModePlotterDefault CONSTANT)
public:
    ~GuiConfiguration() override;

    static GuiConfiguration *instance();

    void setConfigFilePath(const QString &path);
    QString configFilePath() const
    {
        return m_configFile;
    }

    /**
     * Sets the default values for the configuration.
     * Needs to be invoked prior to setting the @link{configFilePath} property.
     * If not present or a feature missing a hardcoded default value is used.
     **/
    Q_INVOKABLE void setDefaultConfigFilePath(const QString &path);

    bool seamSeriesOnProductStructure() const
    {
        return m_seamSeriesOnProductStructure;
    }

    bool seamIntervalsOnProductStructure() const
    {
        return m_seamIntervalsOnProductStructure;
    }

    bool configureBlackLevelOffsetVoltagesOnCamera() const
    {
        return m_configureBlackLevelOffsetVoltagesOnCamera;
    }

    bool configureLinLogOnCamera() const
    {
        return m_configureLinLogOnCamera;
    }

    bool configureThicknessOnSeam() const
    {
        return m_configureThicknessOnSeam;
    }

    bool configureMovingDirectionOnSeam() const
    {
        return m_configureMovingDirectionOnSeam;
    }

    bool ledCalibration() const
    {
        return m_ledCalibration;
    }

    bool quickEditFilterParametersOnSeam() const
    {
        return m_quickEditFilterParametersOnSeam;
    }

    bool qualityFaultCategory2() const
    {
        return m_qualityFaultCategory2;
    }

    bool scalePlotterFromSettings() const
    {
        return m_scalePlotterFromSettings;
    }

    bool formatHardDisk() const
    {
        return m_formatHardDisk;
    }

    uint maximumNumberOfScreenshots() const
    {
        return m_maximumNumberOfScreenshots;
    }

    uint maximumNumberOfSeamsOnOverview() const
    {
        return m_maximumNumberOfSeamsOnOverview;
    }

    static uint defaultMaximumNumberOfScreenshots()
    {
        return 100;
    }

    static uint maximumNumberOfScreenshotsMin()
    {
        return 1;
    }

    static uint maximumNumberOfScreenshotsMax()
    {
        return 1000;
    }

    static uint defaultMaximumNumberOfSeamsOnOverview()
    {
        return 100;
    }

    static uint maximumNumberOfSeamsOnOverviewMin()
    {
        return 1;
    }

    static uint maximumNumberOfSeamsOnOverviewMax()
    {
        return 100;
    }

    static uint numberOfSeamsInPlotterMin()
    {
        return 1u;
    }

    static uint numberOfSeamsInPlotterMax()
    {
        return 100u;
    }

    static uint defaultNumberOfSeamsInPlotter()
    {
        return 1u;
    }

    bool colorSignalsByQuality() const
    {
        return m_colorSignalsByQuality;
    }

    bool displayErrorBoundariesInPlotter() const
    {
        return m_displayErrorBoundariesInPlotter;
    }

    uint numberOfSeamsInPlotter() const
    {
        return m_numberOfSeamsInPlotter;
    }

    uint serialNumberFromExtendedProductInfo()
    {
        return m_serialNumberFromExtendedProductInfo;
    }
    uint serialNumberFromExtendedProductInfoMin()
    {
        return 0;
    }
    uint serialNumberFromExtendedProductInfoMax()
    {
        return 2;
    }
    uint serialNumberFromExtendedProductInfoDefault();

    uint partNumberFromExtendedProductInfo()
    {
        return m_partNumberFromExtendedProductInfo;
    }
    uint partNumberFromExtendedProductInfoMin()
    {
        return 0;
    }
    uint partNumberFromExtendedProductInfoMax()
    {
        return 2;
    }
    uint partNumberFromExtendedProductInfoDefault();

    uint maxTimeLiveModePlotter() const
    {
        return m_maxTimeLiveModePlotter;
    }
    uint maxTimeLiveModePlotterMin() const
    {
        return 1;
    }
    uint constexpr maxTimeLiveModePlotterMax() const
    {
        return 10 * 60;
    }
    uint maxTimeLiveModePlotterDefault() const
    {
        return 15;
    }

    void setSeamSeriesOnProductStructure(bool value);
    void setSeamIntervalsOnProductStructure(bool value);
    void setConfigureBlackLevelOffsetVoltagesOnCamera(bool value);
    void setConfigureLinLogOnCamera(bool value);
    void setConfigureThicknessOnSeam(bool value);
    void setConfigureMovingDirectionOnSeam(bool value);
    void setLedCalibration(bool value);
    void setQuickEditFilterParametersOnSeam(bool value);
    void setQualityFaultCategory2(bool value);
    void setScalePlotterFromSettings(bool value);
    void setFormatHardDisk(bool value);
    void setMaximumNumberOfScreenshots(uint value);
    void setStationName(const QString &stationName);
    void setMaximumNumberOfSeamsOnOverview(uint value);
    void setColorSignalsByQuality(bool value);
    void setDisplayErrorBoundariesInPlotter(bool value);
    void setNumberOfSeamsInPlotter(uint value);
    void setVirtualKeyboard(bool value);
    void setRemoteDesktopOnStartup(bool value);
    void setBlockedAutomatic(bool value);
    void setSerialNumberFromExtendedProductInfo(uint value);
    void setPartNumberFromExtendedProductInfo(uint value);
    void setMaxTimeLiveModePlotter(uint value);

    /**
     * Syncs the current configuration to the config file.
     **/
    Q_INVOKABLE void sync();

    QUuid stationId() const
    {
        return m_stationId;
    }

    QString stationName() const
    {
        return m_stationName;
    }

    QString defaultStationName() const
    {
        return m_stationId.toString(QUuid::WithoutBraces);
    }

    QString roleNameViewUser() const;
    void setRoleNameViewUser(const QString &name);

    QString defaultRoleNameViewUser()
    {
        return QStringLiteral("Operator");
    }

    QStringList availableLanguages() const
    {
        return m_availableLanguages;
    }

    QString language() const
    {
        return m_language;
    }

    QString uiLanguage() const;

    QLocale locale() const
    {
        return m_locale;
    }

    bool virtualKeyboard() const
    {
        return m_virtualKeyboard;
    }

    bool remoteDesktopOnStartup() const
    {
        return m_remoteDesktopOnStartup;
    }

    bool blockedAutomatic() const
    {
        return m_blockedAutomatic;
    }

    int oversamplingRate() const
    {
        return 100;
    }

    void setupTranslators();

    Q_INVOKABLE void retranslate();

Q_SIGNALS:
    void configFilePathChanged();
    void seamSeriesOnProductStructureChanged();
    void seamIntervalsOnProductStructureChanged();
    void configureBlackLevelOffsetVoltagesOnCameraChanged();
    void configureLinLogOnCameraChanged();
    void configureThicknessOnSeamChanged();
    void configureMovingDirectionOnSeamChanged();
    void ledCalibrationChanged();
    void quickEditFilterParametersOnSeamChanged();
    void qualityFaultCategory2Changed();
    void scalePlotterFromSettingsChanged();
    void formatHardDiskChanged();
    void maximumNumberOfScreenshotsChanged();
    void stationNameChanged();
    void maximumNumberOfSeamsOnOverviewChanged();
    void roleNameViewUserChanged();
    void languageChanged();
    void localeChanged();
    void colorSignalsByQualityChanged();
    void displayErrorBoundariesInPlotterChanged();
    void numberOfSeamsInPlotterChanged();
    void virtualKeyboardChanged();
    void remoteDesktopOnStartupChanged();
    void blockedAutomaticChanged();
    void serialNumberFromExtendedProductInfoChanged();
    void partNumberFromExtendedProductInfoChanged();
    void maxTimeLiveModePlotterChanged();

    void translatorsChanged();

private:
    explicit GuiConfiguration();
    void init();
    void initAvailableLanguages();
    void setLanguage(const QString &language);
    QUuid readStationUuid() const;
    bool defaultSeamSeriesOnProductStructure() const;
    bool defaultSeamIntervalsOnProductStructure() const;
    bool defaultConfigureBlackLevelOffsetVoltagesOnCamera() const;
    bool defaultConfigureLinLogOnCamera() const;
    bool defaultConfigureThicknessOnSeam() const;
    bool defaultConfigureMovingDirectionOnSeam() const;
    bool defaultLedCalibration() const;
    bool defaultQuickEditFilterParametersOnSeam() const;
    bool defaultQualityFaultCategory2() const;
    bool defaultScalePlotterFromSettings() const;
    bool defaultFormatHardDisk() const;
    bool defaultColorSignalsByQuality() const;
    bool defaultDisplayErrorBoundariesInPlotter() const;
    bool defaultVirtualKeyboard() const;

    template <typename T>
    QVariant readDefaultValue(const QString &key, T defaultValue) const;

    QString m_configFile;
    QString m_defaultConfigFile;
    bool m_seamSeriesOnProductStructure = false;
    bool m_seamIntervalsOnProductStructure = false;
    bool m_configureBlackLevelOffsetVoltagesOnCamera = false;
    bool m_configureLinLogOnCamera = false;
    bool m_configureThicknessOnSeam = false;
    bool m_configureMovingDirectionOnSeam = false;
    bool m_ledCalibration = false;
    bool m_quickEditFilterParametersOnSeam = false;
    bool m_qualityFaultCategory2 = false;
    bool m_scalePlotterFromSettings = false;
    bool m_formatHardDisk = false;
    bool m_colorSignalsByQuality = false;
    bool m_displayErrorBoundariesInPlotter = true;
    bool m_virtualKeyboard = true;
    bool m_remoteDesktopOnStartup = true;
    bool m_blockedAutomatic = false;
    uint m_maximumNumberOfScreenshots = defaultMaximumNumberOfScreenshots();
    QUuid m_stationId;
    uint m_maximumNumberOfSeamsOnOverview = defaultMaximumNumberOfSeamsOnOverview();
    uint m_numberOfSeamsInPlotter = 1;
    QString m_stationName;
    FileSystemNameValidator *m_stationNameValidator;
    QStringList m_availableLanguages{{QStringLiteral("en")}};
    QString m_language{QStringLiteral("en")};
    QLocale m_locale;
    uint m_serialNumberFromExtendedProductInfo = 0;
    uint m_partNumberFromExtendedProductInfo = 0;
    uint m_maxTimeLiveModePlotter = 15;

    QTranslator *m_translator = nullptr;
    QTranslator *m_corpIdTranslator = nullptr;
    QTranslator *m_headMontiorTranslator = nullptr;
    QTranslator *m_qtTranslator = nullptr;
    QTranslator* m_graphEditorTranslator = nullptr;
    QTranslator* m_figureEditorTranslator = nullptr;
    QTranslator* m_storageTranslator = nullptr;
};

}
}
