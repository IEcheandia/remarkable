#include "weldmasterPaths.h"
#include <QFileInfo>

namespace precitec
{
namespace gui
{

WeldmasterPaths::WeldmasterPaths()
    : QObject()
{
}

WeldmasterPaths::~WeldmasterPaths() = default;

WeldmasterPaths *WeldmasterPaths::instance()
{
    static WeldmasterPaths s_instance;
    return &s_instance;
}

QString WeldmasterPaths::videoBaseDir() const
{
    return baseDirectory() + QStringLiteral("/video/WM-QNX-PC/");
}

QString WeldmasterPaths::resultsBaseDir() const
{
    return baseDirectory() + QStringLiteral("/data/results/");
}

QString WeldmasterPaths::graphDir() const
{
    return configurationDir() + QStringLiteral("graphs/");
}

QString WeldmasterPaths::systemGraphDir() const
{
    return baseDirectory() + QStringLiteral("/system_graphs/");
}

QString WeldmasterPaths::subGraphDir() const
{
    return systemGraphDir() + QStringLiteral("/sub_graphs/");
}

QString WeldmasterPaths::userSubGraphDir() const
{
    return configurationDir() + QStringLiteral("/sub_graphs/");
}

QString WeldmasterPaths::subGraphInputDir() const
{
    return subGraphDir() + QStringLiteral("/1 Input/");
}
QString WeldmasterPaths::subGraphROIDir() const
{
    return subGraphDir() + QStringLiteral("/2 ROI & preSearch/");
}
QString WeldmasterPaths::subGraphPreDir() const
{
    return subGraphDir() + QStringLiteral("/3 Pre-Process/");
}
QString WeldmasterPaths::subGraphProcessDir() const
{
    return subGraphDir() + QStringLiteral("/4 Process/");
}
QString WeldmasterPaths::subGraphPostDir() const
{
    return subGraphDir() + QStringLiteral("/5 Post-Process/");
}
QString WeldmasterPaths::subGraphOutputDir() const
{
    return subGraphDir() + QStringLiteral("/6 Output/");
}
QString WeldmasterPaths::subGraphS6KSpecialS1Dir() const
{
    return subGraphDir() + QStringLiteral("/Special S1/");
}
QString WeldmasterPaths::subGraphS6KSpecialS2Dir() const
{
    return subGraphDir() + QStringLiteral("/Special S2/");
}

QString WeldmasterPaths::assemblyImagesDir() const
{
    return configurationDir() + QStringLiteral("assemblyImages/");
}

QString WeldmasterPaths::configurationDir() const
{
    return baseDirectory() + QStringLiteral("/config/");
}

QString WeldmasterPaths::productDir() const
{
    return configurationDir() + QStringLiteral("products/");
}

QString WeldmasterPaths::logfilesDir() const
{
    return baseDirectory() + QStringLiteral("/logfiles/");
}

QString WeldmasterPaths::updateArchiveDir() const
{
    return baseDirectory() + QStringLiteral("/data/update_archive/");
}

QString WeldmasterPaths::pdfFilesDir() const
{
    return baseDirectory() + QStringLiteral("/pdfs/");
}

QString WeldmasterPaths::hardwareConfigurationBackupDir() const
{
    return baseDirectory() + QStringLiteral("/data/hardware_configuration/");
}

namespace
{
QString initBaseDir()
{
    const QString baseDir{QString::fromUtf8(qgetenv("WM_BASE_DIR"))};
    return QFileInfo{baseDir}.canonicalFilePath();
}
}

QString WeldmasterPaths::baseDirectory() const
{
    static const QString s_baseDir{initBaseDir()};
    return s_baseDir;
}

QString WeldmasterPaths::referenceCruveDir() const
{
    return configurationDir() + QStringLiteral("reference_curves/");
}

QString WeldmasterPaths::laserControlPresetDir() const
{
    return configurationDir() + QStringLiteral("/laser_controls/");
}

QString WeldmasterPaths::openVpnConfigDir() const
{
    return baseDirectory() + QStringLiteral("/data/openvpn/");
}

QString WeldmasterPaths::configurationTemplatesDir() const
{
    return baseDirectory() + QStringLiteral("/config_templates/");
}

QString WeldmasterPaths::scanFieldDir() const
{
    return configurationDir() + QStringLiteral("scanfieldimage/");
}

QString WeldmasterPaths::languageDir() const
{
    return baseDirectory() + QStringLiteral("/languages/");
}

QString WeldmasterPaths::filterLibDir() const
{
    return baseDirectory() + QStringLiteral("/lib/");
}

QString WeldmasterPaths::filterPictureDir() const
{
    return languageDir() + QStringLiteral("filterImages/");
}

QString WeldmasterPaths::referenceImageDir() const
{
    return configurationDir() + QStringLiteral("reference_images/");
}

QString WeldmasterPaths::scanfieldDataDir() const
{
    return baseDirectory() + QStringLiteral("/data/scanfieldimage/");
}

QString WeldmasterPaths::systemMacroDir() const
{
    return systemGraphDir() + QStringLiteral("/macros/");
}

QString WeldmasterPaths::userMacroDir() const
{
    return configurationDir() + QStringLiteral("/macros/");
}

}
}
