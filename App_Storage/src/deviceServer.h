#pragma once
#include "message/device.interface.h"

#include <QString>

namespace precitec
{
namespace storage
{
class ResultsStorageService;

class DeviceServer : public precitec::interface::TDevice<precitec::interface::AbstractInterface>
{
public:
    int initialize(interface::Configuration const& config, int subDevice=0) override;
    void uninitialize() override;
    void reinitialize() override;
    interface::KeyHandle set(interface::SmpKeyValue keyValue, int subDevice=0) override;
    void set(interface::Configuration config, int subDevice=0) override;
    interface::SmpKeyValue get(interface::Key key, int subDevice=0) override;
    interface::SmpKeyValue get(interface::KeyHandle handle, int subDevice=0) override;
    interface::Configuration get(int subDevice=0) override;

    void setResultsStorage(ResultsStorageService *service)
    {
        m_resultsStorage = service;
    }
    ResultsStorageService *resultsStorage() const
    {
        return m_resultsStorage;
    }

    void initFromFile();

    void setConfigFile(const QString &config)
    {
        m_configFile = config;
    }
    QString configFile() const
    {
        return m_configFile;
    }

private:
    interface::SmpKeyValue enabledKey() const;
    interface::SmpKeyValue shutoffKey() const;
    interface::SmpKeyValue diskUsageKey() const;
    interface::SmpKeyValue cacheEntriesKey() const;
    void writeConfig();
    ResultsStorageService *m_resultsStorage = nullptr;
    QString m_configFile;
};


}
}
