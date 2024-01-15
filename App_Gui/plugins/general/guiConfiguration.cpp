#include "guiConfiguration.h"
#include "fileSystemNameValidator.h"
#include "weldmasterPaths.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QLibraryInfo>
#include <QSettings>
#include <QTranslator>

#include <precitec/userManagement.h>

namespace precitec
{
namespace gui
{

using components::user::UserManagement;

GuiConfiguration::GuiConfiguration()
    : QObject()
    , m_stationId(readStationUuid())
    , m_stationName(m_stationId.toString(QUuid::WithoutBraces))
    , m_stationNameValidator(new FileSystemNameValidator{this})
{
    initAvailableLanguages();
    connect(this, &GuiConfiguration::configFilePathChanged, this, &GuiConfiguration::init);
}

GuiConfiguration::~GuiConfiguration() = default;

QUuid GuiConfiguration::readStationUuid() const
{
    QFile machineIdFile{QStringLiteral("/etc/machine-id")};
    if (!machineIdFile.open(QIODevice::ReadOnly))
    {
        return QUuid::createUuid();
    }
    // /etc/machine-id contains hex encoded UUID v4
    // see https://www.freedesktop.org/software/systemd/man/machine-id.html
    const auto machineId = QByteArray::fromHex(machineIdFile.readLine());

    // one should not use it directly, but hash it
    // generating a sha256 containing static key and the random parts of the machine-id
    QCryptographicHash hash{QCryptographicHash::Sha256};
    hash.addData(QByteArrayLiteral("precitec"));
    hash.addData(machineId.left(6));
    hash.addData(machineId.mid(10));
    const auto sha = hash.result();

    // now generating a new UUID matching the v4
    QByteArray generated = sha.left(6);
    generated.append(machineId.mid(6, 4));
    generated.append(sha.mid(10, machineId.length() - 10));
    return QUuid::fromRfc4122(generated);
}

GuiConfiguration *GuiConfiguration::instance()
{
    static GuiConfiguration s_instance;
    return &s_instance;
}

void GuiConfiguration::setConfigFilePath(const QString& path)
{
    if (m_configFile == path)
    {
        return;
    }
    m_configFile = path;
    emit configFilePathChanged();
}

void GuiConfiguration::init()
{
    if (m_configFile.isEmpty())
    {
        return;
    }
    QSettings config{m_configFile, QSettings::IniFormat};
    config.beginGroup(QStringLiteral("Features"));

    setSeamSeriesOnProductStructure(config.value(QStringLiteral("SeamSeriesOnProductStructure"), defaultSeamSeriesOnProductStructure()).toBool());
    setSeamIntervalsOnProductStructure(config.value(QStringLiteral("SeamIntervalsOnProductStructure"), defaultSeamIntervalsOnProductStructure()).toBool());
    setConfigureBlackLevelOffsetVoltagesOnCamera(config.value(QStringLiteral("ConfigureBlackLevelOffsetVoltagesOnCamera"), defaultConfigureBlackLevelOffsetVoltagesOnCamera()).toBool());
    setConfigureLinLogOnCamera(config.value(QStringLiteral("ConfigureLinLogOnCamera"), defaultConfigureLinLogOnCamera()).toBool());
    setConfigureThicknessOnSeam(config.value(QStringLiteral("ConfigureThicknessOnSeam"), defaultConfigureThicknessOnSeam()).toBool());
    setConfigureMovingDirectionOnSeam(config.value(QStringLiteral("ConfigureMovingDirectionOnSeam"), defaultConfigureMovingDirectionOnSeam()).toBool());
    setLedCalibration(config.value(QStringLiteral("LedCalibration"), defaultLedCalibration()).toBool());
    setQuickEditFilterParametersOnSeam(config.value(QStringLiteral("QuickEditFilterParametersOnSeam"), defaultQuickEditFilterParametersOnSeam()).toBool());
    setQualityFaultCategory2(config.value(QStringLiteral("QualityFaultCategory2"), defaultQualityFaultCategory2()).toBool());
    setScalePlotterFromSettings(config.value(QStringLiteral("ScalePlotterFromSettings"),  defaultScalePlotterFromSettings()).toBool());
    setMaximumNumberOfScreenshots(config.value(QStringLiteral("MaximumNumberOfScreenshots"), defaultMaximumNumberOfScreenshots()).toUInt());
    setMaximumNumberOfSeamsOnOverview(config.value(QStringLiteral("MaximumNumberOfSeamsOnOverview"), defaultMaximumNumberOfSeamsOnOverview()).toUInt());
    setColorSignalsByQuality(config.value(QStringLiteral("ColorSignalsByQuality"), defaultColorSignalsByQuality()).toBool());
    setDisplayErrorBoundariesInPlotter(config.value(QStringLiteral("DisplayErrorBoundariesInPlotter"), defaultDisplayErrorBoundariesInPlotter()).toBool());
    setNumberOfSeamsInPlotter(config.value(QStringLiteral("NumberOfSeamsInPlotter"), defaultNumberOfSeamsInPlotter()).toUInt());
    setSerialNumberFromExtendedProductInfo(config.value(QStringLiteral("SerialNumberFromExtendedProductInfo"), serialNumberFromExtendedProductInfoDefault()).toUInt());
    setPartNumberFromExtendedProductInfo(config.value(QStringLiteral("PartNumberFromExtendedProductInfo"), partNumberFromExtendedProductInfoDefault()).toUInt());
    setMaxTimeLiveModePlotter(config.value(QStringLiteral("MaxTimeLiveModePlotter"), maxTimeLiveModePlotter()).toUInt());

    setStationName(config.value(QStringLiteral("StationName"), m_stationId.toString(QUuid::WithoutBraces)).toString());
    setRoleNameViewUser(config.value(QStringLiteral("RoleNameViewUser"), defaultRoleNameViewUser()).toString());
    setVirtualKeyboard(config.value(QStringLiteral("VirtualKeyboard"), defaultVirtualKeyboard()).toBool());
    setRemoteDesktopOnStartup(config.value(QStringLiteral("RemoteDesktopOnStartup"), false).toBool());
    setBlockedAutomatic(config.value(QStringLiteral("BlockedAutomatic"), false).toBool());

    config.endGroup();

    config.beginGroup(QStringLiteral("UI"));
    setLanguage(config.value(QStringLiteral("Language"), m_availableLanguages.first()).toString());
    if (config.contains(QStringLiteral("Locale")))
    {
        m_locale = config.value(QStringLiteral("Locale"), QLocale::system()).toLocale();
        QLocale::setDefault(m_locale);
        emit localeChanged();
    }
    config.endGroup();
}

void GuiConfiguration::initAvailableLanguages()
{
    const auto languageFiles = QDir{WeldmasterPaths::instance()->languageDir()}.entryList({QStringLiteral("weldmaster.*.qm")}, QDir::Files);
    std::transform(languageFiles.begin(), languageFiles.end(), std::back_inserter(m_availableLanguages), [] (const QString &file) { return file.split(QLatin1String(".")).at(1); });
}

void GuiConfiguration::sync()
{
    if (m_configFile.isEmpty())
    {
        return;
    }
    QSettings config{m_configFile, QSettings::IniFormat};
    config.beginGroup(QStringLiteral("Features"));

    config.setValue(QStringLiteral("SeamSeriesOnProductStructure"), m_seamSeriesOnProductStructure);
    config.setValue(QStringLiteral("SeamIntervalsOnProductStructure"), m_seamIntervalsOnProductStructure);
    config.setValue(QStringLiteral("ConfigureBlackLevelOffsetVoltagesOnCamera"), m_configureBlackLevelOffsetVoltagesOnCamera);
    config.setValue(QStringLiteral("ConfigureLinLogOnCamera"), m_configureLinLogOnCamera);
    config.setValue(QStringLiteral("ConfigureThicknessOnSeam"), m_configureThicknessOnSeam);
    config.setValue(QStringLiteral("ConfigureMovingDirectionOnSeam"), m_configureMovingDirectionOnSeam);
    config.setValue(QStringLiteral("LedCalibration"), m_ledCalibration);
    config.setValue(QStringLiteral("QuickEditFilterParametersOnSeam"), m_quickEditFilterParametersOnSeam);
    config.setValue(QStringLiteral("QualityFaultCategory2"), m_qualityFaultCategory2);
    config.setValue(QStringLiteral("ScalePlotterFromSettings"),  m_scalePlotterFromSettings);
    config.setValue(QStringLiteral("MaximumNumberOfScreenshots"), m_maximumNumberOfScreenshots);
    config.setValue(QStringLiteral("StationName"), m_stationName);
    config.setValue(QStringLiteral("MaximumNumberOfSeamsOnOverview"), m_maximumNumberOfSeamsOnOverview);
    config.setValue(QStringLiteral("RoleNameViewUser"), roleNameViewUser());
    config.setValue(QStringLiteral("ColorSignalsByQuality"), m_colorSignalsByQuality);
    config.setValue(QStringLiteral("DisplayErrorBoundariesInPlotter"), m_displayErrorBoundariesInPlotter);
    config.setValue(QStringLiteral("NumberOfSeamsInPlotter"), m_numberOfSeamsInPlotter);
    config.setValue(QStringLiteral("VirtualKeyboard"), m_virtualKeyboard);
    config.setValue(QStringLiteral("RemoteDesktopOnStartup"), m_remoteDesktopOnStartup);
    config.setValue(QStringLiteral("BlockedAutomatic"), m_blockedAutomatic );
    config.setValue(QStringLiteral("SerialNumberFromExtendedProductInfo"), m_serialNumberFromExtendedProductInfo);
    config.setValue(QStringLiteral("PartNumberFromExtendedProductInfo"), m_partNumberFromExtendedProductInfo);
    config.setValue(QStringLiteral("MaxTimeLiveModePlotter"), m_maxTimeLiveModePlotter);

    config.endGroup();
    config.sync();
}


QString GuiConfiguration::roleNameViewUser() const
{
    return UserManagement::instance()->roleModel()->index(0, 0).data(Qt::UserRole + 1).toString();
}

void GuiConfiguration::setRoleNameViewUser(const QString& name)
{
    if (name == roleNameViewUser())
    {
        return;
    }
    if (auto role = UserManagement::instance()->roleModel()->index(0, 0).data(Qt::UserRole).value<QObject*>())
    {
        role->setProperty("visibleName", name);
        emit roleNameViewUserChanged();
    }
}

void GuiConfiguration::setSeamSeriesOnProductStructure(bool value)
{
    if (m_seamSeriesOnProductStructure == value)
    {
        return;
    }
    m_seamSeriesOnProductStructure = value;
    emit seamSeriesOnProductStructureChanged();
}

void GuiConfiguration::setSeamIntervalsOnProductStructure(bool value)
{
    if (m_seamIntervalsOnProductStructure == value)
    {
        return;
    }
    m_seamIntervalsOnProductStructure = value;
    emit seamIntervalsOnProductStructureChanged();
}

void GuiConfiguration::setConfigureBlackLevelOffsetVoltagesOnCamera(bool value)
{
    if (m_configureBlackLevelOffsetVoltagesOnCamera == value)
    {
        return;
    }
    m_configureBlackLevelOffsetVoltagesOnCamera = value;
    emit configureBlackLevelOffsetVoltagesOnCameraChanged();
}

void GuiConfiguration::setConfigureLinLogOnCamera(bool value)
{
    if (m_configureLinLogOnCamera == value)
    {
        return;
    }
    m_configureLinLogOnCamera = value;
    emit configureLinLogOnCameraChanged();
}

void GuiConfiguration::setConfigureThicknessOnSeam(bool value)
{
    if (m_configureThicknessOnSeam == value)
    {
        return;
    }
    m_configureThicknessOnSeam = value;
    emit configureThicknessOnSeamChanged();
}

void GuiConfiguration::setConfigureMovingDirectionOnSeam(bool value)
{
    if (m_configureMovingDirectionOnSeam == value)
    {
        return;
    }
    m_configureMovingDirectionOnSeam = value;
    emit configureMovingDirectionOnSeamChanged();
}

void GuiConfiguration::setLedCalibration(bool value)
{
    if (m_ledCalibration == value)
    {
        return;
    }
    m_ledCalibration = value;
    emit ledCalibrationChanged();
}

void GuiConfiguration::setQuickEditFilterParametersOnSeam(bool value)
{
    if (m_quickEditFilterParametersOnSeam == value)
    {
        return;
    }
    m_quickEditFilterParametersOnSeam = value;
    emit quickEditFilterParametersOnSeamChanged();
}

void GuiConfiguration::setQualityFaultCategory2(bool value)
{
    if (m_qualityFaultCategory2 == value)
    {
        return;
    }
    m_qualityFaultCategory2 = value;
    emit qualityFaultCategory2Changed();
}

void GuiConfiguration::setScalePlotterFromSettings(bool value)
{
    if (m_scalePlotterFromSettings == value)
    {
        return;
    }
    m_scalePlotterFromSettings = value;
    emit scalePlotterFromSettingsChanged();
}

void GuiConfiguration::setFormatHardDisk(bool value)
{
    if (m_formatHardDisk == value)
    {
        return;
    }
    m_formatHardDisk = value;
    emit formatHardDiskChanged();
}

void GuiConfiguration::setColorSignalsByQuality(bool value)
{
    if (m_colorSignalsByQuality == value)
    {
        return;
    }
    m_colorSignalsByQuality = value;
    emit colorSignalsByQualityChanged();
}

void GuiConfiguration::setDisplayErrorBoundariesInPlotter(bool value)
{
    if (m_displayErrorBoundariesInPlotter == value)
    {
        return;
    }
    m_displayErrorBoundariesInPlotter = value;
    emit displayErrorBoundariesInPlotterChanged();
}

void GuiConfiguration::setNumberOfSeamsInPlotter(uint value)
{
    if (m_numberOfSeamsInPlotter == value)
    {
        return;
    }
    m_numberOfSeamsInPlotter = value;
    emit numberOfSeamsInPlotterChanged();
}

void GuiConfiguration::setMaximumNumberOfScreenshots(uint value)
{
    if (m_maximumNumberOfScreenshots == value)
    {
        return;
    }
    m_maximumNumberOfScreenshots = value;
    emit maximumNumberOfScreenshotsChanged();
}

void GuiConfiguration::setStationName(const QString& stationName)
{
    QString sanitized = stationName;
    m_stationNameValidator->fixup(sanitized);
    int pos = 0;
    if (m_stationNameValidator->validate(sanitized, pos) != QValidator::Acceptable)
    {
        sanitized = m_stationId.toString(QUuid::WithoutBraces);
    }
    if (m_stationName == sanitized)
    {
        return;
    }
    m_stationName = sanitized;
    emit stationNameChanged();
}

void GuiConfiguration::setMaximumNumberOfSeamsOnOverview(uint value)
{
    if (m_maximumNumberOfSeamsOnOverview == value)
    {
        return;
    }
    m_maximumNumberOfSeamsOnOverview = value;
    emit maximumNumberOfSeamsOnOverviewChanged();
}

void GuiConfiguration::setLanguage(const QString& language)
{
    if (m_language == language)
    {
        return;
    }
    m_language = language;
    emit languageChanged();
}

void GuiConfiguration::setDefaultConfigFilePath(const QString &path)
{
    m_defaultConfigFile = path;
}

void GuiConfiguration::setVirtualKeyboard(bool value)
{
    if (m_virtualKeyboard == value)
    {
        return;
    }
    m_virtualKeyboard = value;
    emit virtualKeyboardChanged();
}

void GuiConfiguration::setRemoteDesktopOnStartup(bool value)
{
    if (m_remoteDesktopOnStartup == value)
    {
        return;
    }
    m_remoteDesktopOnStartup = value;
    emit remoteDesktopOnStartupChanged();
}

void GuiConfiguration::setBlockedAutomatic(bool value)
{
    if ( m_blockedAutomatic == value)
    {
        return;
    }
    m_blockedAutomatic = value;
    emit blockedAutomaticChanged();
}

template <typename T>
QVariant GuiConfiguration::readDefaultValue(const QString &key, T defaultValue) const
{
    if (m_defaultConfigFile.isEmpty())
    {
        return defaultValue;
    }
    QSettings config{m_defaultConfigFile, QSettings::IniFormat};
    return config.value(QStringLiteral("Features/") + key, defaultValue);
}

bool GuiConfiguration::defaultSeamSeriesOnProductStructure() const
{
    return readDefaultValue(QStringLiteral("SeamSeriesOnProductStructure"), false).toBool();
}

bool GuiConfiguration::defaultSeamIntervalsOnProductStructure() const
{
    return readDefaultValue(QStringLiteral("SeamIntervalsOnProductStructure"), false).toBool();
}

bool GuiConfiguration::defaultConfigureBlackLevelOffsetVoltagesOnCamera() const
{
    return readDefaultValue(QStringLiteral("ConfigureBlackLevelOffsetVoltagesOnCamera"), false).toBool();
}

bool GuiConfiguration::defaultConfigureLinLogOnCamera() const
{
    return readDefaultValue(QStringLiteral("ConfigureLinLogOnCamera"), false).toBool();
}

bool GuiConfiguration::defaultConfigureThicknessOnSeam() const
{
    return readDefaultValue(QStringLiteral("ConfigureThicknessOnSeam"), false).toBool();
}

bool GuiConfiguration::defaultConfigureMovingDirectionOnSeam() const
{
    return readDefaultValue(QStringLiteral("ConfigureMovingDirectionOnSeam"), false).toBool();
}

bool GuiConfiguration::defaultLedCalibration() const
{
    return readDefaultValue(QStringLiteral("LedCalibration"), false).toBool();
}

bool GuiConfiguration::defaultQuickEditFilterParametersOnSeam() const
{
    return readDefaultValue(QStringLiteral("QuickEditFilterParametersOnSeam"), false).toBool();
}

bool GuiConfiguration::defaultQualityFaultCategory2() const
{
    return readDefaultValue(QStringLiteral("QualityFaultCategory2"), false).toBool();
}

bool GuiConfiguration::defaultScalePlotterFromSettings() const
{
    return readDefaultValue(QStringLiteral("ScalePlotterFromSettings"), false).toBool();
}

bool GuiConfiguration::defaultFormatHardDisk() const
{
    return readDefaultValue(QStringLiteral("FormatHardDisk"), false).toBool();
}

bool GuiConfiguration::defaultColorSignalsByQuality() const
{
    return readDefaultValue(QStringLiteral("ColorSignalsByQuality"), false).toBool();
}

bool GuiConfiguration::defaultDisplayErrorBoundariesInPlotter() const
{
    return readDefaultValue(QStringLiteral("DisplayErrorBoundariesInPlotter"), true).toBool();
}

bool GuiConfiguration::defaultVirtualKeyboard() const
{
    return readDefaultValue(QStringLiteral("VirtualKeyboard"), true).toBool();
}

uint GuiConfiguration::serialNumberFromExtendedProductInfoDefault()
{
    return readDefaultValue(QStringLiteral("SerialNumberFromExtendedProductInfo"), 0u).toUInt();
}

uint GuiConfiguration::partNumberFromExtendedProductInfoDefault()
{
    return readDefaultValue(QStringLiteral("PartNumberFromExtendedProductInfo"), 0u).toUInt();
}

void GuiConfiguration::setupTranslators()
{
    m_translator = new QTranslator{this};
    if (m_translator->load(QLocale{language()}, QStringLiteral("weldmaster"), QStringLiteral("."), WeldmasterPaths::instance()->languageDir(), QStringLiteral(".qm")))
    {
        QCoreApplication::instance()->installTranslator(m_translator);
    }

    m_graphEditorTranslator = new QTranslator{this};
    if (m_graphEditorTranslator->load(QLocale{language()}, QStringLiteral("weldmaster-grapheditor"), QStringLiteral("_"), WeldmasterPaths::instance()->languageDir(), QStringLiteral(".qm")))
    {
        QCoreApplication::instance()->installTranslator(m_graphEditorTranslator);
    }

    m_figureEditorTranslator = new QTranslator{this};
    if (m_figureEditorTranslator->load(QLocale{language()}, QStringLiteral("weldmaster-figureditor"), QStringLiteral("_"), WeldmasterPaths::instance()->languageDir(), QStringLiteral(".qm")))
    {
        QCoreApplication::instance()->installTranslator(m_figureEditorTranslator);
    }

    m_storageTranslator = new QTranslator{this};
    if (m_storageTranslator->load(QLocale{language()}, QStringLiteral("weldmaster-storage"), QStringLiteral("_"), WeldmasterPaths::instance()->languageDir(), QStringLiteral(".qm")))
    {
        QCoreApplication::instance()->installTranslator(m_storageTranslator);
    }

    m_corpIdTranslator = new QTranslator{this};
    if (m_corpIdTranslator->load(QLocale{language()}, QStringLiteral("precitecCorporateIdElements"), QStringLiteral("_"), QStringLiteral("/usr/share/precitec-corporate-id-components/translations/")))
    {
        QCoreApplication::instance()->installTranslator(m_corpIdTranslator);
    }

    m_headMontiorTranslator = new QTranslator{this};
    if (m_headMontiorTranslator->load(QLocale{language()}, QStringLiteral("precitecHeadMonitor"), QStringLiteral("_"), QStringLiteral("/usr/share/precitec-headmonitor/translations/")))
    {
        QCoreApplication::instance()->installTranslator(m_headMontiorTranslator);
    }

    m_qtTranslator = new QTranslator{this};
    if (m_qtTranslator->load("qt_" + language(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
    {
        QCoreApplication::instance()->installTranslator(m_qtTranslator);
    }
}

void GuiConfiguration::retranslate()
{
    if (m_corpIdTranslator)
    {
        QCoreApplication::instance()->removeTranslator(m_corpIdTranslator);
        delete m_corpIdTranslator;
        m_corpIdTranslator = nullptr;
    }
    if (m_qtTranslator)
    {
        QCoreApplication::instance()->removeTranslator(m_qtTranslator);
        delete m_qtTranslator;
        m_qtTranslator = nullptr;
    }
    if (m_translator)
    {
        QCoreApplication::instance()->removeTranslator(m_translator);
        delete m_translator;
        m_translator = nullptr;
    }
    if (m_headMontiorTranslator)
    {
        QCoreApplication::instance()->removeTranslator(m_headMontiorTranslator);
        delete m_headMontiorTranslator;
        m_headMontiorTranslator = nullptr;
    }
    if (m_graphEditorTranslator)
    {
        QCoreApplication::instance()->removeTranslator(m_graphEditorTranslator);
        delete m_graphEditorTranslator;
        m_graphEditorTranslator = nullptr;
    }
    if (m_figureEditorTranslator)
    {
        QCoreApplication::instance()->removeTranslator(m_figureEditorTranslator);
        delete m_figureEditorTranslator;
        m_figureEditorTranslator = nullptr;
    }
    if (m_storageTranslator)
    {
        QCoreApplication::instance()->removeTranslator(m_storageTranslator);
        delete m_storageTranslator;
        m_storageTranslator = nullptr;
    }
    init();
    setupTranslators();
    emit translatorsChanged();
}

void GuiConfiguration::setSerialNumberFromExtendedProductInfo(uint value)
{
    if (m_serialNumberFromExtendedProductInfo == value)
    {
        return;
    }
    m_serialNumberFromExtendedProductInfo = value;
    emit serialNumberFromExtendedProductInfoChanged();
}

void GuiConfiguration::setPartNumberFromExtendedProductInfo(uint value)
{
    if (m_partNumberFromExtendedProductInfo == value)
    {
        return;
    }
    m_partNumberFromExtendedProductInfo = value;
    emit partNumberFromExtendedProductInfoChanged();
}

void GuiConfiguration::setMaxTimeLiveModePlotter(uint value)
{
    if (m_maxTimeLiveModePlotter == value)
    {
        return;
    }
    m_maxTimeLiveModePlotter = value;
    emit maxTimeLiveModePlotterChanged();
}

QString GuiConfiguration::uiLanguage() const
{
    const QLocale locale{language()};
    // we need the language code like de-DE or en-US. This is in the uiLanguages of the locale.
    const auto &uiLanguages = locale.uiLanguages();
    if (auto it = std::find_if(uiLanguages.begin(), uiLanguages.end(), [] (const auto &language) { return language.count(QChar('-')) == 1; }); it != uiLanguages.end())
    {
        return *it;
    }
    return language();
}

}
}
