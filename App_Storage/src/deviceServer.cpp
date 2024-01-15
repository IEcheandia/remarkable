#include "deviceServer.h"
#include "resultsStorageService.h"
#include "module/moduleLogger.h"
#include <QtGlobal>

#include <Poco/File.h>
#include <Poco/Util/XMLConfiguration.h>

namespace precitec
{
namespace storage
{

static const std::string s_enabled{"ResultsStorageEnabled"};
static const std::string s_maxRelativeStorage("ResultsRelativeDiskUsage");
static const std::string s_maxCacheEntries("ResultsCacheEntries");
static const std::string s_shutoff("ResultsShutOff");

int DeviceServer::initialize(interface::Configuration const& config, int subDevice)
{
    Q_UNUSED(config)
    Q_UNUSED(subDevice)
    wmLog(eWarning, "precitec::storage::DeviceServer::initialize not implemented");
    return -1;
}

void DeviceServer::uninitialize()
{
    wmLog(eWarning, "precitec::storage::DeviceServer::uninitialize not implemented");
}

void DeviceServer::reinitialize()
{
    wmLog(eWarning, "precitec::storage::DeviceServer::reinitialize not implemented");
}

interface::KeyHandle DeviceServer::set(interface::SmpKeyValue keyValue, int subDevice)
{
    Q_UNUSED(subDevice)
    if (!m_resultsStorage)
    {
        return {};
    }
    if (!keyValue)
    {
        return {};
    }
    bool updated = false;
    if (keyValue->key() == s_enabled)
    {
        updated = true;
        m_resultsStorage->setEnabled(keyValue->value<bool>());
    } else if (keyValue->key() == s_maxRelativeStorage)
    {
        updated = true;
        m_resultsStorage->setMaxRelativeDiskUsage(keyValue->value<double>());
    } else if (keyValue->key() == s_maxCacheEntries)
    {
        updated = true;
        m_resultsStorage->setMaxCacheEntries(keyValue->value<uint>());
    }
    if (updated)
    {
        writeConfig();
        return {1};
    }
    return {};
}

void DeviceServer::set(interface::Configuration config, int subDevice)
{
    if (!m_resultsStorage)
    {
        return;
    }
    for (auto kv : config)
    {
        set(kv, subDevice);
    }
}

interface::SmpKeyValue DeviceServer::get(interface::Key key, int subDevice)
{
    Q_UNUSED(subDevice)
    if (!m_resultsStorage)
    {
        return interface::SmpKeyValue(new interface::KeyValue(TInt, "?", -1));
    }
    if (key == s_enabled)
    {
        return enabledKey();
    } else if (key == s_maxRelativeStorage)
    {
        return diskUsageKey();
    } else if (key == s_maxCacheEntries)
    {
        return cacheEntriesKey();
    } else if (key == s_shutoff)
    {
        return shutoffKey();
    }
    return interface::SmpKeyValue(new interface::KeyValue(TInt, "?", -1));
}

interface::SmpKeyValue DeviceServer::get(interface::KeyHandle handle, int subDevice)
{
    Q_UNUSED(handle)
    Q_UNUSED(subDevice)
    return interface::SmpKeyValue(new interface::KeyValue(TInt, "?", -1));
}

interface::Configuration DeviceServer::get(int subDevice)
{
    Q_UNUSED(subDevice)
    if (!m_resultsStorage)
    {
        return {};
    }
    return {
        enabledKey(),
        diskUsageKey(),
        cacheEntriesKey(),
        shutoffKey()
    };
}

interface::SmpKeyValue DeviceServer::enabledKey() const
{
    return interface::SmpKeyValue(new interface::TKeyValue<bool>(s_enabled, m_resultsStorage->isEnabled(), false, true, true));
}

interface::SmpKeyValue DeviceServer::shutoffKey() const
{
    auto kv = interface::SmpKeyValue(new interface::TKeyValue<bool>(s_shutoff, m_resultsStorage->isShutdown(), false, true, false));
    kv->setReadOnly(true);
    return kv;
}

interface::SmpKeyValue DeviceServer::diskUsageKey() const
{
    return interface::SmpKeyValue(new interface::TKeyValue<double>(s_maxRelativeStorage, m_resultsStorage->maxRelativeDiskUsage(), 0.0, 1.0, 0.9));
}

interface::SmpKeyValue DeviceServer::cacheEntriesKey() const
{
    return interface::SmpKeyValue(new interface::TKeyValue<uint>(s_maxCacheEntries, m_resultsStorage->maxCacheEntries(), 0, 999999, 500));
}

void DeviceServer::initFromFile()
{
    if (m_configFile.isEmpty())
    {
        return;
    }
    Poco::File file{m_configFile.toStdString()};
    if (!file.exists())
    {
        return;
    }
    Poco::AutoPtr<Poco::Util::XMLConfiguration> config;
    try
    { // poco syntax exception might be thrown or sax parse excpetion
        config = new Poco::Util::XMLConfiguration(file.path());
    }
    catch (const Poco::Exception &p_rException)
    {
        wmLog(eDebug, "%s - '%s': %s\n", __FUNCTION__, p_rException.name(), p_rException.message().c_str());
        wmLog(eDebug, "Could not read parameters from file:\n");
        wmLog(eDebug, "'%s'.\n", file.path().c_str());
        wmLogTr(eWarning, "QnxMsg.Vdr.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, p_rException.message().c_str());
    }
    if (!config)
    {
        return;
    }

    if (config->has(s_enabled))
    {
        auto keyValue = enabledKey()->clone();
        keyValue->setValue(config->getBool(keyValue->key(), keyValue->defValue<bool>()));
        set(keyValue);
    }
    if (config->has(s_maxRelativeStorage))
    {
        auto keyValue = diskUsageKey()->clone();
        keyValue->setValue(config->getDouble(keyValue->key(), keyValue->defValue<double>()));
        set(keyValue);
    }
    if (config->has(s_maxCacheEntries))
    {
        auto keyValue = cacheEntriesKey()->clone();
        keyValue->setValue(config->getUInt(keyValue->key(), keyValue->defValue<uint>()));
        set(keyValue);
    }
}

void DeviceServer::writeConfig()
{
    if (m_configFile.isEmpty())
    {
        return;
    }
    Poco::File file{m_configFile.toStdString()};
    try
    {
        file.createFile();
    } catch(...)
    {
        wmLog(eWarning, "Failed to create config file %s\n", m_configFile.toStdString());
        return;
    }
    interface::writeToFile(m_configFile.toStdString(), get());
}

}
}
