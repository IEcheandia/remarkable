#pragma once
#include "keyValueConfigUpdater.h"

namespace precitec
{

class NutConfigUpdater
{
public:
    enum class Ups
    {
        None,
        ABBPowerValue11RT,
        APCSmartUps,
        ABBPowerValue11RTG2,
        OmronS8BA24D24D120LF_ser,
        OmronS8BA24D24D120LF_usb,
    };
    NutConfigUpdater(Ups ups, const QString &baseDir = QStringLiteral("/etc/nut/"));

    void operator()()
    {
        m_nutConf.execute();
        if (m_ups != Ups::None)
        {
            m_upsConf.execute();
        }
    }

private:
    Ups m_ups;
    KeyValueConfigUpdater m_nutConf;
    KeyValueConfigUpdater m_upsConf;
};

}

