#include "deviceServer.h"
#include "deviceNotificationServer.h"
#include "guiConfiguration.h"
#include <QtGlobal>
#include <QMetaProperty>
#include <QVariant>
#include <QUuid>

#include <functional>

namespace precitec
{
namespace gui
{

static const std::vector<std::string> s_strings = {
    std::string{"stationId"},
    std::string{"stationName"},
    std::string{"roleNameViewUser"}
};

static const std::vector<std::string> s_booleans = {
    std::string{"seamSeriesOnProductStructure"},
    std::string{"seamIntervalsOnProductStructure"},
    std::string{"configureBlackLevelOffsetVoltagesOnCamera"},
    std::string{"configureLinLogOnCamera"},
    std::string{"configureThicknessOnSeam"},
    std::string{"configureMovingDirectionOnSeam"},
    std::string{"ledCalibration"},
    std::string{"quickEditFilterParametersOnSeam"},
    std::string{"qualityFaultCategory2"},
    std::string{"scalePlotterFromSettings"},
    std::string{"formatHardDisk"},                  //formatHardDisk is showGraphEditor and showProductionSetup
    std::string{"colorSignalsByQuality"},
    std::string{"displayErrorBoundariesInPlotter"},
    std::string{"virtualKeyboard"},
    std::string{"remoteDesktopOnStartup"},
    std::string{"blockedAutomatic"},
};

static const std::vector<std::string> s_uints = {
    std::string{"maximumNumberOfScreenshots"},
    std::string{"maximumNumberOfSeamsOnOverview"},
    std::string{"numberOfSeamsInPlotter"},
    std::string{"serialNumberFromExtendedProductInfo"},
    std::string{"partNumberFromExtendedProductInfo"},
    std::string{"maxTimeLiveModePlotter"},
};

static const QUuid s_guiID{QByteArrayLiteral("7b27d004-47c9-4f9a-b807-cae94806ae67")};

DeviceServer::DeviceServer()
    : interface::TDevice<interface::AbstractInterface>()
{
}

DeviceServer::~DeviceServer() = default;

int DeviceServer::initialize(interface::Configuration const& config, int subDevice)
{
    Q_UNUSED(config)
    Q_UNUSED(subDevice)
    return 0;
}

void DeviceServer::uninitialize()
{
}

void DeviceServer::reinitialize()
{
}

namespace
{

template <typename T>
interface::TKeyValue<T> *kv(const std::string &key);

template <>
interface::TKeyValue<bool> *kv(const std::string &key)
{
    const auto index = GuiConfiguration::instance()->metaObject()->indexOfProperty(key.c_str());
    if (index == -1)
    {
        return nullptr;
    }
    auto property = GuiConfiguration::instance()->metaObject()->property(index);
    auto value = new interface::TKeyValue<bool>(key, property.read(GuiConfiguration::instance()).toBool(), false, true, false);
    if (property.isConstant())
    {
        value->setReadOnly(true);
    }
    return value;
}

template <>
interface::TKeyValue<std::string> *kv(const std::string &key)
{
    auto g = GuiConfiguration::instance();
    const auto index = g->metaObject()->indexOfProperty(key.c_str());
    if (index == -1)
    {
        return nullptr;
    }
    auto property = g->metaObject()->property(index);
    auto value = new interface::TKeyValue<std::string>(key, property.read(g).toString().toStdString(), {}, {}, g->property(std::string{key + std::string{"Default"}}.c_str()).toString().toStdString());
    if (property.isConstant())
    {
        value->setReadOnly(true);
    }
    return value;
}

template <>
interface::TKeyValue<uint> *kv(const std::string &key)
{
    auto g = GuiConfiguration::instance();
    const auto index = g->metaObject()->indexOfProperty(key.c_str());
    if (index == -1)
    {
        return nullptr;
    }
    auto property = g->metaObject()->property(index);
    auto value = new interface::TKeyValue<uint>(key, property.read(g).toUInt(),
                                                g->property(std::string{key + std::string{"Min"}}.c_str()).toUInt(),
                                                g->property(std::string{key + std::string{"Max"}}.c_str()).toUInt(),
                                                g->property(std::string{key + std::string{"Default"}}.c_str()).toUInt());
    if (property.isConstant())
    {
        value->setReadOnly(true);
    }
    return value;
}

}

interface::KeyHandle DeviceServer::set(interface::SmpKeyValue keyValue, int subDevice)
{
    Q_UNUSED(subDevice)
    auto it = std::find(s_booleans.begin(), s_booleans.end(), keyValue->key());
    interface::SmpKeyValue updatedValue;
    if (it != s_booleans.end())
    {
        GuiConfiguration::instance()->setProperty(it->c_str(), keyValue->value<bool>());
        updatedValue = kv<bool>(*it);
    }
    it = std::find(s_uints.begin(), s_uints.end(), keyValue->key());
    if (it != s_uints.end())
    {
        GuiConfiguration::instance()->setProperty(it->c_str(), keyValue->value<uint>());
        updatedValue = kv<uint>(*it);
    }
    it = std::find(s_strings.begin(), s_strings.end(), keyValue->key());
    if (it != s_strings.end())
    {
        GuiConfiguration::instance()->setProperty(it->c_str(), QString::fromStdString(keyValue->value<std::string>()));
        updatedValue = kv<std::string>(*it);
    }
    if (!updatedValue.isNull())
    {
        GuiConfiguration::instance()->sync();
        if (m_notificationServer)
        {
            emit m_notificationServer->changed(s_guiID, updatedValue);
        }
    }
    return {};
}

void DeviceServer::set(interface::Configuration config, int subDevice)
{
    Q_UNUSED(config)
    Q_UNUSED(subDevice)
}

interface::SmpKeyValue DeviceServer::get(interface::Key key, int subDevice)
{
    Q_UNUSED(subDevice)
    return value(key);
}

interface::SmpKeyValue DeviceServer::get(interface::KeyHandle handle, int subDevice)
{
    Q_UNUSED(handle)
    Q_UNUSED(subDevice)
    return {};
}

interface::Configuration DeviceServer::get(int subDevice)
{
    Q_UNUSED(subDevice)
    interface::Configuration configuration;
    configuration.reserve(s_strings.size() + s_booleans.size() + s_uints.size());
    for (const auto &key : s_strings)
    {
        configuration.emplace_back(kv<std::string>(key));
    }
    for (const auto &key : s_booleans)
    {
        configuration.emplace_back(kv<bool>(key));
    }
    for (const auto &key : s_uints)
    {
        configuration.emplace_back(kv<uint>(key));
    }
    return configuration;
}

interface::SmpKeyValue DeviceServer::value(const std::string &name) const
{
    auto it = std::find(s_booleans.begin(), s_booleans.end(), name);
    if (it != s_booleans.end())
    {
        return interface::SmpKeyValue(kv<bool>(*it));
    }
    it = std::find(s_uints.begin(), s_uints.end(), name);
    if (it != s_uints.end())
    {
        return interface::SmpKeyValue(kv<uint>(*it));
    }
    it = std::find(s_strings.begin(), s_strings.end(), name);
    if (it != s_strings.end())
    {
        return interface::SmpKeyValue(kv<std::string>(*it));
    }
    return {};
}

void DeviceServer::setNotificationServer(DeviceNotificationServer *server)
{
    m_notificationServer = server;
}

}
}
