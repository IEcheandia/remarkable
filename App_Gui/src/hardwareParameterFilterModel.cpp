#include "hardwareParameterFilterModel.h"
#include "deviceProxyWrapper.h"
#include "parameter.h"
#include "parameterSet.h"

#include <QtConcurrentRun>
#include <QFutureWatcher>

using precitec::interface::Configuration;
using precitec::storage::Parameter;
using precitec::storage::ParameterSet;

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

HardwareParameterFilterModel::HardwareParameterFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &HardwareParameterFilterModel::filterKeysChanged, this, &HardwareParameterFilterModel::invalidateFilter);
    connect(this, &HardwareParameterFilterModel::deviceProxyChanged, this, &HardwareParameterFilterModel::invalidateFilter);
    connect(this, &HardwareParameterFilterModel::ledTypeChanged, this, &HardwareParameterFilterModel::invalidateFilter);
}

HardwareParameterFilterModel::~HardwareParameterFilterModel() = default;

void HardwareParameterFilterModel::setFilterKeys(const QVariantList &filterKeys)
{
    m_filterKeys.clear();
    for (auto key: filterKeys)
    {
        m_filterKeys << key.value<HardwareParameters::Key>();
    }
    emit filterKeysChanged();
}

QVariantList HardwareParameterFilterModel::filterKeys() const
{
    QVariantList ret;
    for (auto key: m_filterKeys)
    {
        ret << QVariant::fromValue(key);
    }
    return ret;
}

bool HardwareParameterFilterModel::filterLEDKey(HardwareParameters::Key key) const
{
    switch (key)
    {
    case HardwareParameters::Key::LEDPanel8Intensity:
    case HardwareParameters::Key::LEDPanel8OnOff:
    case HardwareParameters::Key::LEDPanel8PulseWidth:
    case HardwareParameters::Key::LEDPanel7Intensity:
    case HardwareParameters::Key::LEDPanel7OnOff:
    case HardwareParameters::Key::LEDPanel7PulseWidth:
    case HardwareParameters::Key::LEDPanel6Intensity:
    case HardwareParameters::Key::LEDPanel6OnOff:
    case HardwareParameters::Key::LEDPanel6PulseWidth:
    case HardwareParameters::Key::LEDPanel5Intensity:
    case HardwareParameters::Key::LEDPanel5OnOff:
    case HardwareParameters::Key::LEDPanel5PulseWidth:
        return m_ledType != 3;
    case HardwareParameters::Key::LEDPanel4Intensity:
    case HardwareParameters::Key::LEDPanel4OnOff:
    case HardwareParameters::Key::LEDPanel4PulseWidth:
    case HardwareParameters::Key::LEDPanel3Intensity:
    case HardwareParameters::Key::LEDPanel3OnOff:
    case HardwareParameters::Key::LEDPanel3PulseWidth:
        return m_ledType != 1 && m_ledType != 3;
    case HardwareParameters::Key::LEDPanel2Intensity:
    case HardwareParameters::Key::LEDPanel2OnOff:
    case HardwareParameters::Key::LEDPanel2PulseWidth:
    case HardwareParameters::Key::LEDPanel1Intensity:
    case HardwareParameters::Key::LEDPanel1OnOff:
    case HardwareParameters::Key::LEDPanel1PulseWidth:
        return m_ledType != 2 && m_ledType != 1 && m_ledType != 3;
    default:
        return false;
    }
}

bool HardwareParameterFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    auto index = sourceModel()->index(source_row, 0, source_parent);
    const auto variant = index.data(Qt::UserRole);
    if (!variant.isValid())
    {
        return false;
    }
    if (m_filterKeys.empty())
    {
        return true;
    }
    const auto key = variant.value<HardwareParameters::Key>();
    if (m_filterKeys.contains(key))
    {
        if (filterLEDKey(key))
        {
            return false;
        }

        const auto& properties = HardwareParameters::instance()->properties(key);
        // check in configuration for the key
        auto deviceKeyIt = std::find_if(m_deviceConfiguration.begin(), m_deviceConfiguration.end(), [&properties] (auto keyValue) { return keyValue->key() == HardwareParameters::instance()->deviceKey(properties.device); } );
        if (deviceKeyIt == m_deviceConfiguration.end())
        {
            // not in configuration -> disable
            return false;
        }

        return (*deviceKeyIt)->value<bool>();
    }
    return false;
}

DeviceProxyWrapper *HardwareParameterFilterModel::deviceProxy() const
{
    return m_deviceProxy.data();
}

void HardwareParameterFilterModel::setDeviceProxy(DeviceProxyWrapper *deviceProxy)
{
    if (m_deviceProxy.data() == deviceProxy)
    {
        return;
    }
    m_deviceProxy = deviceProxy;
    if (m_deviceProxy)
    {
        auto watcher = new QFutureWatcher<Configuration>{this};
        connect(watcher, &QFutureWatcher<Configuration>::finished, this,
            [this, watcher]
            {
                watcher->deleteLater();
                m_deviceConfiguration = watcher->result();
                invalidateFilter();
            }
        );
        watcher->setFuture(QtConcurrent::run(
            [this]
            {
                return m_deviceProxy->deviceProxy()->get();
            }
        ));
    }
    emit deviceProxyChanged();
}

DeviceProxyWrapper *HardwareParameterFilterModel::weldHeadDeviceProxy() const
{
    return m_weldHeadDeviceProxy.data();
}

void HardwareParameterFilterModel::setWeldHeadDeviceProxy(DeviceProxyWrapper *deviceProxy)
{
    if (m_weldHeadDeviceProxy.data() == deviceProxy)
    {
        return;
    }
    m_weldHeadDeviceProxy = deviceProxy;
    if (m_weldHeadDeviceProxy)
    {
        auto watcher = new QFutureWatcher<Configuration>{this};
        connect(watcher, &QFutureWatcher<Configuration>::finished, this,
            [this, watcher]
            {
                watcher->deleteLater();
                auto configuration = watcher->result();

                setLedType(m_weldHeadDeviceProxy->deviceProxy()->get(std::string("LED_CONTROLLER_TYPE"), 0)->value<int>());
            }
        );
        watcher->setFuture(QtConcurrent::run(
            [this]
            {
                return m_weldHeadDeviceProxy->deviceProxy()->get();
            }
        ));
    }
    emit weldHeadDeviceProxyChanged();
}

void HardwareParameterFilterModel::filterPrameterSet(ParameterSet *parameterSet) const
{
    // get all parameters
    std::vector<Parameter *> parameters;
    parameters.reserve(rowCount());

    for (int i = 0; i < rowCount(); i++)
    {
        if (auto p = index(i, 0).data(Qt::UserRole + 3).value<Parameter *>())
        {
            parameters.push_back(p);
        }
    }
    // check which parameters to remove
    std::vector<Parameter *> toRemove;
    for (auto p: parameterSet->parameters())
    {
        if (std::none_of(parameters.begin(), parameters.end(), [p] (auto test) { return test->name() == p->name(); }))
        {
            toRemove.push_back(p);
        }
    }

    // remove parameters
    for (auto p: toRemove)
    {
        parameterSet->removeParameter(p);
    }
}

void HardwareParameterFilterModel::setLedType(int type)
{
    if (m_ledType == type)
    {
        return;
    }
    m_ledType = type;
    emit ledTypeChanged();
}

}
}
