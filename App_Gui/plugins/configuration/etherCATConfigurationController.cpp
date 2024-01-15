#include "etherCATConfigurationController.h"
#include "etherCATConfigurationChangeEntry.h"

#include <QFileInfo>
#include <QProcess>

#include <precitec/userLog.h>

#include <Poco/Util/PropertyFileConfiguration.h>

namespace precitec
{
namespace gui
{

using components::userLog::UserLog;

EtherCATConfigurationController::EtherCATConfigurationController(QObject *parent)
    : QObject(parent)
    , m_macAddress(initMacAddress())
    , m_enabled(initEnabled())
{
    connect(this, &EtherCATConfigurationController::macAddressChanged, this, &EtherCATConfigurationController::markAsModified);
    connect(this, &EtherCATConfigurationController::enabledChanged, this, &EtherCATConfigurationController::markAsModified);
}

EtherCATConfigurationController::~EtherCATConfigurationController() = default;

QByteArray EtherCATConfigurationController::initMacAddress() const
{
    QFileInfo ethercatFile{QStringLiteral("/etc/ethercat.conf")};
    if (!ethercatFile.exists())
    {
        return {};
    }
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> settings{new Poco::Util::PropertyFileConfiguration{ethercatFile.absoluteFilePath().toStdString()}};
    return QByteArray::fromStdString(settings->getString("MASTER0_DEVICE")).replace('"', QByteArray{});
}

bool EtherCATConfigurationController::initEnabled()
{
    const std::string startupConfigFile{std::string(getenv("WM_BASE_DIR")) + std::string("/system_config/StandaloneStartup.config")};
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> startupConfig{new Poco::Util::PropertyFileConfiguration{startupConfigFile}};

    return startupConfig->getBool(std::string("EtherCATMaster.Enabled"), true);
}

void EtherCATConfigurationController::setMacAddress(const QByteArray &macAddress)
{
    if (m_macAddress == macAddress)
    {
        return;
    }
    m_macAddress = macAddress;
    emit macAddressChanged();
}

void EtherCATConfigurationController::markAsModified()
{
    if (m_modified)
    {
        return;
    }
    m_modified = true;
    emit modifiedChanged();
}

void EtherCATConfigurationController::save()
{
    if (!m_modified)
    {
        return;
    }
    const bool wasEnabled = initEnabled();
    const QByteArray oldMacAddress = initMacAddress();
    UserLog::instance()->addChange(new EtherCATConfigurationChangeEntry{wasEnabled, m_enabled, oldMacAddress, m_macAddress});

    QStringList arguments;
    if (wasEnabled != m_enabled)
    {
        if (m_enabled)
        {
            arguments << QStringLiteral("--enable");
        } else
        {
            arguments << QStringLiteral("--disable");
        }
    }
    if (oldMacAddress != m_macAddress)
    {
        arguments << QStringLiteral("--macAddress");
        arguments << QString::fromLocal8Bit(m_macAddress);
    }

    QProcess::startDetached(QString::fromUtf8(qgetenv("WM_BASE_DIR")) + QStringLiteral("/bin/etherCATConfigurationHelper"), arguments);

    m_modified = false;
    emit modifiedChanged();
}

void EtherCATConfigurationController::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
    {
        return;
    }
    m_enabled = enabled;
    emit enabledChanged();
}

}
}
