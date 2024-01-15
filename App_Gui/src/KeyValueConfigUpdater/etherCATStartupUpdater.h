#pragma once
#include "keyValueConfigUpdater.h"

namespace precitec
{

class EtherCATStartupUpdater
{
public:
    EtherCATStartupUpdater(bool enable);
    void operator()()
    {
        m_startupConfig.execute();
        m_connectConfig.execute();
    }

private:
    KeyValueConfigUpdater m_startupConfig;
    KeyValueConfigUpdater m_connectConfig;
};

}
