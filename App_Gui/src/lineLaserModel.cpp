#include "lineLaserModel.h"
#include "deviceProxyWrapper.h"
#include "message/device.proxy.h"

#include <QFutureWatcher>
#include <QtConcurrentRun>

using namespace precitec::interface;

namespace precitec
{
namespace gui
{

LineLaserModel::LineLaserModel(QObject *parent)
    : AbstractDeviceKeyModel(parent)
{
    connect(this, &AbstractDeviceKeyModel::deviceProxyChanged, this, &LineLaserModel::init);
    connect(this, &AbstractDeviceKeyModel::notificationServerChanged, this,
        [this]
        {
            disconnect(m_notificationConnection);
            if (auto s = notificationServer())
            {
                m_notificationConnection = connect(s, &DeviceNotificationServer::changed, this, &LineLaserModel::keyValueChanged, Qt::QueuedConnection);
            } else
            {
                m_notificationConnection = QMetaObject::Connection{};
            }
        }
    );
}

LineLaserModel::~LineLaserModel() = default;

int LineLaserModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_data.size();
}

QVariant LineLaserModel::data(const QModelIndex &index, int role) const
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
        return m_data.at(index.row()).updating;
    }
    return {};
}

QHash<int, QByteArray> LineLaserModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::UserRole, QByteArrayLiteral("enabled")},
        {Qt::UserRole + 1, QByteArrayLiteral("intensity")},
        {Qt::UserRole + 2, QByteArrayLiteral("minimum")},
        {Qt::UserRole + 3, QByteArrayLiteral("maximum")},
        {Qt::UserRole + 4, QByteArrayLiteral("updating")}
    };
}

static const std::vector<std::tuple<std::string, std::string, std::string>> s_referenceKeys{
    {std::string{"LineLaser1OnOff"}, std::string{"LineLaser1Intensity"}, std::string{QT_TR_NOOP("Line Laser 1")}},
    {std::string{"LineLaser2OnOff"}, std::string{"LineLaser2Intensity"}, std::string{QT_TR_NOOP("Line Laser 2")}},
    {std::string{"FieldLight1OnOff"}, std::string{"FieldLight1Intensity"}, std::string{QT_TR_NOOP("Line Laser 3")}}
};

void LineLaserModel::init()
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
            beginResetModel();
            const auto config = watcher->result();
            for (const auto &element : s_referenceKeys)
            {
                auto enableIt = std::find_if(config.begin(), config.end(), [element] (auto kv) { return kv->key() == std::get<0>(element); });
                auto intensityIt = std::find_if(config.begin(), config.end(), [element] (auto kv) { return kv->key() == std::get<1>(element); });
                if (enableIt == config.end() || intensityIt == config.end())
                {
                    continue;
                }
                m_data.emplace_back(Data{*enableIt, *intensityIt, tr(std::get<2>(element).c_str())});
            }
            endResetModel();
        }
    );
    watcher->setFuture(QtConcurrent::run(deviceProxy()->deviceProxy().get(), &TDevice<AbstractInterface>::get, 0));
}

void LineLaserModel::enable(int index, bool enable)
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
    QtConcurrent::run([kv, this] { deviceProxy()->setKeyValue(kv); });
}

void LineLaserModel::setIntensity(int index, int intensity)
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
    m_data.at(index).updating = true;
    emit dataChanged(this->index(index, 0), this->index(index, 0), {Qt::UserRole + 4});
    kv->setValue(intensity);
    QtConcurrent::run([kv, this] { deviceProxy()->setKeyValue(kv); });
}

void LineLaserModel::keyValueChanged(const QUuid &id, const precitec::interface::SmpKeyValue &kv)
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
            m_data.at(i).updating = false;
            emit dataChanged(index(i, 0), index(i, 0), {Qt::UserRole + 1, Qt::UserRole + 4});
            break;
        }
    }
}

}
}
