#include "fieldIlluminationModel.h"
#include "deviceProxyWrapper.h"
#include "message/device.proxy.h"
#include <iostream>

#include <QFutureWatcher>
#include <QtConcurrentRun>

using namespace precitec::interface;

namespace precitec
{
namespace gui
{

namespace
{

template <typename T>
T getValue(const Configuration &configuration, const std::string &key, T defaultValue)
{
    auto it = std::find_if(configuration.begin(), configuration.end(), [key] (auto kv) { return kv->key() == key; });
    if (it == configuration.end())
    {
        return defaultValue;
    }
    return (*it)->template value<T>();
}

}

FieldIlluminationModel::FieldIlluminationModel(QObject *parent)
    : AbstractDeviceKeyModel(parent)
{
    connect(this, &AbstractDeviceKeyModel::deviceProxyChanged, this, &FieldIlluminationModel::init);
    connect(this, &AbstractDeviceKeyModel::notificationServerChanged, this,
        [this]
        {
            disconnect(m_notificationConnection);
            if (auto s = notificationServer())
            {
               m_notificationConnection = connect(s, &DeviceNotificationServer::changed, this, &FieldIlluminationModel::keyValueChanged, Qt::QueuedConnection);
            } else
            {
               m_notificationConnection = QMetaObject::Connection{};
            }
        }
    );
}

FieldIlluminationModel::~FieldIlluminationModel() = default;

int FieldIlluminationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_data.size();
}

QVariant FieldIlluminationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    switch (role)
    {
    case Qt::DisplayRole:
        return m_data.at(index.row()).name;
    case Qt::UserRole:
        return m_data.at(index.row()).toggle->value<bool>();
    case Qt::UserRole + 1:
        return m_data.at(index.row()).intensity->value<int>();
    case Qt::UserRole + 2:
        return m_data.at(index.row()).intensity->minima<int>();
    case Qt::UserRole + 3:
        return m_data.at(index.row()).intensity->maxima<int>();
    case Qt::UserRole + 4:
        return m_data.at(index.row()).pulseWidth->value<int>();
    case Qt::UserRole + 5:
        return m_data.at(index.row()).pulseWidth->minima<int>();
    case Qt::UserRole + 6:
        return m_data.at(index.row()).pulseWidth->maxima<int>();
    }
    return {};
}

QHash<int, QByteArray> FieldIlluminationModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::UserRole, QByteArrayLiteral("enabled")},
        {Qt::UserRole + 1, QByteArrayLiteral("intensity")},
        {Qt::UserRole + 2, QByteArrayLiteral("minimumIntensity")},
        {Qt::UserRole + 3, QByteArrayLiteral("maximumIntensity")},
        {Qt::UserRole + 4, QByteArrayLiteral("pulsWidth")},
        {Qt::UserRole + 5, QByteArrayLiteral("minimumPulsWidth")},
        {Qt::UserRole + 6, QByteArrayLiteral("maximumPulsWidth")}
   };
}


static const std::vector<std::tuple<std::string, std::string, std::string, std::string>> s_referenceKeys{
    {std::string{"LEDPanel1OnOff"}, std::string{"LEDPanel1Intensity"}, std::string{"LEDPanel1PulseWidth"}, std::string{QT_TR_NOOP("LED Panel 1")}},
    {std::string{"LEDPanel2OnOff"}, std::string{"LEDPanel2Intensity"}, std::string{"LEDPanel2PulseWidth"}, std::string{QT_TR_NOOP("LED Panel 2")}},
    {std::string{"LEDPanel3OnOff"}, std::string{"LEDPanel3Intensity"}, std::string{"LEDPanel3PulseWidth"}, std::string{QT_TR_NOOP("LED Panel 3")}},
    {std::string{"LEDPanel4OnOff"}, std::string{"LEDPanel4Intensity"}, std::string{"LEDPanel4PulseWidth"}, std::string{QT_TR_NOOP("LED Panel 4")}},
    {std::string{"LEDPanel5OnOff"}, std::string{"LEDPanel5Intensity"}, std::string{"LEDPanel5PulseWidth"}, std::string{QT_TR_NOOP("LED Panel 5")}},
    {std::string{"LEDPanel6OnOff"}, std::string{"LEDPanel6Intensity"}, std::string{"LEDPanel6PulseWidth"}, std::string{QT_TR_NOOP("LED Panel 6")}},
    {std::string{"LEDPanel7OnOff"}, std::string{"LEDPanel7Intensity"}, std::string{"LEDPanel7PulseWidth"}, std::string{QT_TR_NOOP("LED Panel 7")}},
    {std::string{"LEDPanel8OnOff"}, std::string{"LEDPanel8Intensity"}, std::string{"LEDPanel8PulseWidth"}, std::string{QT_TR_NOOP("LED Panel 8")}},
};

void FieldIlluminationModel::init()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
    if (!deviceProxy())
    {
       return;
    }

    auto *watcher = new QFutureWatcher<Configuration>(this);
    connect(watcher, &QFutureWatcher<Configuration>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            if (!deviceProxy())
            {
                return;
            }
            beginResetModel();
            const auto config = watcher->result();

            const auto ledType = deviceProxy()->deviceProxy()->get(Key("LED_CONTROLLER_TYPE"), 0)->value<int>();

            if (ledType != 0)
            {
                auto i = 0;
                for (const auto &element : s_referenceKeys)
                {
                    if (i > 1 && ledType != 1 && ledType != 3)
                    {
                        break;
                    }
                    if (i > 3 && ledType != 3)
                    {
                        break;
                    }
                    auto enableIt = std::find_if(config.begin(), config.end(), [element] (auto kv) { return kv->key() == std::get<0>(element); });
                    auto intensityIt = std::find_if(config.begin(), config.end(), [element] (auto kv) { return kv->key() == std::get<1>(element); });
                    auto pulseWidthIt = std::find_if(config.begin(), config.end(), [element] (auto kv) { return kv->key() == std::get<2>(element); });
                    if (enableIt == config.end() || intensityIt == config.end() || pulseWidthIt == config.end())
                    {
                        continue;
                    }
                    m_data.emplace_back(Data{*enableIt, *intensityIt, *pulseWidthIt, tr(std::get<3>(element).c_str())});
                    i++;
                }
            }
            endResetModel();
        }
    );
    watcher->setFuture(QtConcurrent::run(deviceProxy()->deviceProxy().get(), &TDevice<AbstractInterface>::get, 0));
}

void FieldIlluminationModel::enable(int index, bool enable)
{
    if (!deviceProxy())
    {
        return;
    }
    if (std::size_t(index) >= m_data.size())
    {
        return;
    }
    auto kv = m_data.at(index).toggle->clone();
    kv->setValue(enable);
    QtConcurrent::run([kv, this]
    {
        QMutexLocker lock{proxyMutex()};
        if (auto proxy = deviceProxy())
        {
            proxy->setKeyValue(kv);
            proxy->setKeyValue(SmpKeyValue{new TKeyValue<bool>{std::string{"LEDSendData"}, true}});
        }
    });
}

void FieldIlluminationModel::setIntensity(int index, int intensity)
{
    if (!deviceProxy())
    {
        return;
    }
    if (std::size_t(index) >= m_data.size())
    {
        return;
    }
    auto kv = m_data.at(index).intensity->clone();
    kv->setValue(intensity);
    QtConcurrent::run([kv, this]
    {
        QMutexLocker lock{proxyMutex()};
        if (auto proxy = deviceProxy())
        {
            proxy->setKeyValue(kv);
            proxy->setKeyValue(SmpKeyValue{new TKeyValue<bool>{std::string{"LEDSendData"}, true}});
        }
    });
}

void FieldIlluminationModel::setPulseWidth(int index, int pulseWidth)
{
    if (!deviceProxy())
    {
        return;
    }
    if (std::size_t(index) >= m_data.size())
    {
        return;
    }
    auto kv = m_data.at(index).pulseWidth->clone();
    kv->setValue(pulseWidth);
    QtConcurrent::run([kv, this]
    {
        QMutexLocker lock{proxyMutex()};
        if (auto proxy = deviceProxy())
        {
            proxy->setKeyValue(kv);
            proxy->setKeyValue(SmpKeyValue{new TKeyValue<bool>{std::string{"LEDSendData"}, true}});
        }
    });
}

void FieldIlluminationModel::keyValueChanged(const QUuid &id, const precitec::interface::SmpKeyValue &kv)
{
    if (!deviceProxy() || deviceProxy()->uuid() != id)
    {
        return;
    }
    for (std::size_t i = 0; i < m_data.size(); i++)
    {
        if (m_data.at(i).toggle->key() == kv->key())
        {
            m_data.at(i).toggle= kv;
            emit dataChanged(index(i, 0), index(i, 0), {Qt::UserRole});
            break;
        } else if (m_data.at(i).intensity->key() == kv->key())
        {
            m_data.at(i).intensity = kv;
            emit dataChanged(index(i, 0), index(i, 0), {Qt::UserRole + 1});
            break;
        } else if (m_data.at(i).pulseWidth->key() == kv->key())
        {
            m_data.at(i).pulseWidth = kv;
            emit dataChanged(index(i, 0), index(i, 0), {Qt::UserRole + 4});
            break;
        }
    }
}

}
}

