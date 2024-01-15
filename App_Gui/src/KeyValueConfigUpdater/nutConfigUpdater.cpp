#include "nutConfigUpdater.h"

namespace precitec
{

NutConfigUpdater::NutConfigUpdater(Ups ups, const QString &baseDir)
    : m_ups(ups)
{
    m_nutConf.setUseSpaces(false);
    m_nutConf.setFilePath(baseDir + QStringLiteral("nut.conf"));
    m_upsConf.setFilePath(baseDir + QStringLiteral("ups.conf"));

    QByteArray driver;
    QByteArray port;
    QByteArray description;
    QByteArray mode;
    QByteArray offdelay;
    QByteArray ondelay;
    switch (ups)
    {
    case Ups::None:
        mode = QByteArrayLiteral("none");
        break;
    case Ups::ABBPowerValue11RT:
        driver = QByteArrayLiteral("blazer_usb");
        port = QByteArrayLiteral("auto");
        description = QByteArrayLiteral("\"ABB Powervalue 11RT\"");
        mode = QByteArrayLiteral("standalone");
        offdelay = QByteArrayLiteral("60");
        ondelay = QByteArrayLiteral("0");
        break;
    case Ups::APCSmartUps:
        driver = QByteArrayLiteral("usbhid-ups");
        port = QByteArrayLiteral("auto");
        description = QByteArrayLiteral("\"APC Smart-ups\"");
        mode = QByteArrayLiteral("standalone");
        offdelay = QByteArrayLiteral("30");
        ondelay = QByteArrayLiteral("60");
        break;
    case Ups::ABBPowerValue11RTG2:
        driver = QByteArrayLiteral("nutdrv_qx");
        port = QByteArrayLiteral("auto");
        description = QByteArrayLiteral("\"ABB Powervalue 11RT G2\"");
        mode = QByteArrayLiteral("standalone");
        offdelay = QByteArrayLiteral("30");
        ondelay = QByteArrayLiteral("60");
        break;
    case Ups::OmronS8BA24D24D120LF_ser: // for Omron UPS via serial port
        driver = QByteArrayLiteral("blazer_ser");
        port = QByteArrayLiteral("/dev/ttyS0");
        description = QByteArrayLiteral("\"Omron S8BA24D24D120LF\"");
        mode = QByteArrayLiteral("standalone");
        offdelay = QByteArrayLiteral("60");
        ondelay = QByteArrayLiteral("0");
        break;
    case Ups::OmronS8BA24D24D120LF_usb: // for Omron UPS via USB
        driver = QByteArrayLiteral("blazer_usb");
        port = QByteArrayLiteral("auto");
        description = QByteArrayLiteral("\"Omron S8BA24D24D120LF\"");
        mode = QByteArrayLiteral("standalone");
        offdelay = QByteArrayLiteral("60");
        ondelay = QByteArrayLiteral("0");
        break;
    }

    m_nutConf.addKeyValue(QByteArrayLiteral("MODE"), mode);
    m_upsConf.addKeyValue(QByteArrayLiteral("    driver"), driver);
    m_upsConf.addKeyValue(QByteArrayLiteral("    port"), port);
    m_upsConf.addKeyValue(QByteArrayLiteral("    desc"), description);
    // for Omron UPS via USB
    if (ups == Ups::OmronS8BA24D24D120LF_usb)
    {
        m_upsConf.addKeyValue(QByteArrayLiteral("    subdriver"), QByteArrayLiteral("omron"));
        m_upsConf.addKeyValue(QByteArrayLiteral("    vendorid"), QByteArrayLiteral("0590"));
        m_upsConf.addKeyValue(QByteArrayLiteral("    productid"), QByteArrayLiteral("00c1"));
    }
    else
    {
        m_upsConf.addKeyValue(QByteArrayLiteral("    subdriver"), QByteArrayLiteral("!DELETE!"));
        m_upsConf.addKeyValue(QByteArrayLiteral("    vendorid"), QByteArrayLiteral("!DELETE!"));
        m_upsConf.addKeyValue(QByteArrayLiteral("    productid"), QByteArrayLiteral("!DELETE!"));
    }
    m_upsConf.addKeyValue(QByteArrayLiteral("    offdelay"), offdelay);
    m_upsConf.addKeyValue(QByteArrayLiteral("    ondelay"), ondelay);

}

}

