#include "etherCATStartupUpdater.h"

namespace precitec
{

EtherCATStartupUpdater::EtherCATStartupUpdater(bool enable)
{
    m_startupConfig.setFilePath(QString::fromLocal8Bit(qgetenv("WM_BASE_DIR")) + QStringLiteral("/system_config/StandaloneStartup.config"));
    m_connectConfig.setFilePath(QString::fromLocal8Bit(qgetenv("WM_BASE_DIR")) + QStringLiteral("/system_config/StandaloneConnect.config"));

    const QByteArray value = enable ? QByteArrayLiteral("True") : QByteArrayLiteral("False");

    m_startupConfig.addKeyValue(QByteArrayLiteral("EtherCATMaster.Enabled"), value);
    m_startupConfig.addKeyValue(QByteArrayLiteral("StartEthercat.Enabled"), value);
    m_connectConfig.addKeyValue(QByteArrayLiteral("ECatMaster.Watch"), value);
}

}
