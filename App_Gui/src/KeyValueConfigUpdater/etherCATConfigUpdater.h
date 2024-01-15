#pragma once
#include "keyValueConfigUpdater.h"

namespace precitec
{

class EtherCATConfigUpdater
{
public:
    EtherCATConfigUpdater(const QByteArray &macAddress, const QString &baseDir = QStringLiteral("/etc/"));

    void operator()()
    {
        m_config.execute();
    }

private:
    KeyValueConfigUpdater m_config;
};

}
