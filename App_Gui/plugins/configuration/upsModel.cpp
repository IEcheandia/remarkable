#include "upsModel.h"
#include "upsConfigurationChangeEntry.h"

#include <QDir>
#include <QProcess>
#include <QSettings>

#include <precitec/userLog.h>

namespace precitec
{
namespace gui
{

using components::userLog::UserLog;

UpsModel::UpsModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_upses({
        {Mode::None, Driver::None, tr("No UPS"), QString{}},
        {Mode::Standalone, Driver::BlazerUsb, QLatin1String("ABB Powervalue 11RT"), QLatin1String("ABB PowerValue 11 RT.jpg")},
        {Mode::Standalone, Driver::UsbhidUps, QLatin1String("APC Smart-ups"), QLatin1String("APC Smart-ups 750 VA LCD RM 2U 120 V.jpg")},
        {Mode::Standalone, Driver::NutdrvQx, QLatin1String("ABB PowerValue 11RT G2"), QLatin1String("ABB PowerValue 11 RT G2.png")},
        //{Mode::Standalone, Driver::BlazerSerOmron, QLatin1String("Omron S8BA-24D24D120LF"), QLatin1String("Omron_S8BA-24D24D120LF.png")} // for Omron UPS via serial port
        {Mode::Standalone, Driver::BlazerUsbOmron, QLatin1String("Omron S8BA-24D24D120LF"), QLatin1String("Omron_S8BA-24D24D120LF.png")} // for Omron UPS via USB
    })
    , m_mode(initMode())
    , m_driver(initDriver())
    , m_selectedIndex(indexForConfiguration())
{
    connect(this, &UpsModel::nutConfigDirChanged, this,
        [this]
        {
            m_mode = initMode();
            m_driver = initDriver();
            m_selectedIndex = indexForConfiguration();
            emit selectedIndexChanged();
        }
    );
}

UpsModel::~UpsModel() = default;

int UpsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_upses.size();
}

QVariant UpsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    switch (role)
    {
    case Qt::DisplayRole:
        return m_upses.at(index.row()).description;
    case Qt::UserRole:
        return int(m_upses.at(index.row()).mode);
    case Qt::UserRole + 1:
        return int(m_upses.at(index.row()).driver);
    case Qt::DecorationRole:
        return m_upses.at(index.row()).image;
    }
    return {};
}

UpsModel::Mode UpsModel::initMode() const
{
    QFile nutConf{QDir{m_nutConfigDir}.filePath(QLatin1String("nut.conf"))};
    if (!nutConf.exists())
    {
        return Mode::None;
    }
    if (!nutConf.open(QIODevice::ReadOnly))
    {
        return Mode::None;
    }
    while (true)
    {
        QByteArray line = nutConf.readLine();
        if (line.isEmpty())
        {
            break;
        }
        if (line.startsWith('#'))
        {
            continue;
        }
        if (line.startsWith(QByteArrayLiteral("MODE")))
        {
            auto parts{line.trimmed().split('=')};
            if (parts.size() < 2)
            {
                continue;
            }
            if (qstrcmp(parts.at(1).toLower(), QByteArrayLiteral("none")) == 0)
            {
                return Mode::None;
            }
            if (qstrcmp(parts.at(1).toLower(), QByteArrayLiteral("standalone")) == 0)
            {
                return Mode::Standalone;
            }
        }
    }

    return Mode::None;
}

UpsModel::Driver UpsModel::initDriver() const
{
    if (m_mode == Mode::None)
    {
        return Driver::None;
    }

    QSettings settings{QDir{m_nutConfigDir}.filePath(QLatin1String("ups.conf")), QSettings::IniFormat};
    QString driver = settings.value(QLatin1String("myups/driver")).toString();
    if (driver.compare(QLatin1String("blazer_usb"), Qt::CaseInsensitive) == 0)
    {
        QString desc = settings.value(QStringLiteral("myups/desc")).toString();
        if (desc.compare(QLatin1Literal("Omron S8BA24D24D120LF"), Qt::CaseInsensitive) == 0)
        {
            return Driver::BlazerUsbOmron;
        }
        else
        {
            return Driver::BlazerUsb;
        }
    }
    if (driver.compare(QLatin1String("usbhid-ups"), Qt::CaseInsensitive) == 0)
    {
        return Driver::UsbhidUps;
    }
    if (driver.compare(QLatin1String("nutdrv_qx"), Qt::CaseInsensitive) == 0)
    {
        return Driver::NutdrvQx;
    }
    if (driver.compare(QLatin1Literal("blazer_ser"), Qt::CaseInsensitive) == 0)
    {
        return Driver::BlazerSerOmron;
    }
    return Driver::None;
}

void UpsModel::setNutConfigDir(const QString& config)
{
    if (m_nutConfigDir == config)
    {
        return;
    }
    m_nutConfigDir = config;
    emit nutConfigDirChanged();
}

QModelIndex UpsModel::indexForConfiguration() const
{
    auto it = std::find_if(m_upses.begin(), m_upses.end(), [this] (const auto &ups) { return m_mode == ups.mode && m_driver == ups.driver; });
    if (it == m_upses.end())
    {
        return {};
    }
    return index(std::distance(m_upses.begin(), it), 0);
}

void UpsModel::select(int row)
{
    auto i = index(row, 0);
    if (!i.isValid())
    {
        return;
    }
    if (i == m_selectedIndex)
    {
        return;
    }
    m_selectedIndex = i;
    markAsModified();
    emit selectedIndexChanged();
}

void UpsModel::selectByModeAndDriver(int mode, int driver)
{
    m_mode = Mode(mode);
    m_driver = Driver(driver);
    const auto index = indexForConfiguration();
    if (!index.isValid())
    {
        return;
    }
    if (m_selectedIndex != index)
    {
        m_selectedIndex = index;
        markAsModified();
        emit selectedIndexChanged();
    }
}

void UpsModel::markAsModified()
{
    if (m_modified)
    {
        return;
    }
    m_modified = true;
    modifiedChanged();
}

void UpsModel::save()
{
    if (!m_modified)
    {
        return;
    }
    bool isEnabled = false;

    QStringList arguments;
    if (m_upses.at(m_selectedIndex.row()).mode == Mode::None)
    {
        arguments << QLatin1String("--none");
    } else if (m_upses.at(m_selectedIndex.row()).driver == Driver::BlazerUsb)
    {
        arguments << QLatin1String("--blazerusb");
        isEnabled = true;
    } else if (m_upses.at(m_selectedIndex.row()).driver == Driver::UsbhidUps)
    {
        arguments << QLatin1String("--usbhidups");
        isEnabled = true;
    } else if (m_upses.at(m_selectedIndex.row()).driver == Driver::NutdrvQx)
    {
        arguments << QLatin1String("--nutdrvqx");
        isEnabled = true;
    } else if (m_upses.at(m_selectedIndex.row()).driver == Driver::BlazerSerOmron)
    {
        arguments << QStringLiteral("--blazerserOmron");
        isEnabled = true;
    } else if (m_upses.at(m_selectedIndex.row()).driver == Driver::BlazerUsbOmron)
    {
        arguments << QStringLiteral("--blazerusbOmron");
        isEnabled = true;
    } else
    {
        return;
    }
    UserLog::instance()->addChange(new UpsConfigurationChangeEntry{m_mode == Mode::Standalone, isEnabled, indexForConfiguration().data().toString(), m_selectedIndex.data().toString()});

    QProcess::startDetached(QString::fromUtf8(qgetenv("WM_BASE_DIR")) + QLatin1String("/bin/nutConfigurationHelper"), arguments);

    m_modified = false;
    emit modifiedChanged();
}

}
}
