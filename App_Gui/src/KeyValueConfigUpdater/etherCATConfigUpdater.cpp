#include "etherCATConfigUpdater.h"

namespace precitec
{

EtherCATConfigUpdater::EtherCATConfigUpdater(const QByteArray &macAddress, const QString &baseDir)
{
    m_config.setUseSpaces(false);
    m_config.setFilePath(baseDir + QStringLiteral("ethercat.conf"));
    m_config.setFallbackFilePath(QStringLiteral("/opt/etherlab/etc/ethercat.conf"));
    m_config.addKeyValue(QByteArrayLiteral("MASTER0_DEVICE"), QByteArrayLiteral("\"") + macAddress + QByteArrayLiteral("\""));
    m_config.addKeyValue(QByteArrayLiteral("DEVICE_MODULES"), QByteArrayLiteral("\"generic\""));
}

}
