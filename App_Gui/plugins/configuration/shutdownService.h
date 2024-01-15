#pragma once

#include <QObject>

namespace precitec
{
namespace gui
{

/**
 * A service class which can send management requests to the ConnectServer.
 **/
class ShutdownService : public QObject
{
    Q_OBJECT
    /**
     * The permission to shutdown the system, default is @c -1
     **/
    Q_PROPERTY(int shutdownSystemPermission READ shutdownSystemPermission WRITE setShutdownSystemPermission NOTIFY shutdownSystemPermissionChanged)
    /**
     * The permission to restart the system, default is @c -1
     **/
    Q_PROPERTY(int restartSystemPermission READ restartSystemPermission WRITE setRestartSystemPermission NOTIFY restartSystemPermissionChanged)
    /**
     * The permission to stop all processes, default is @c -1
     **/
    Q_PROPERTY(int stopAllProcessesPermission READ stopAllProcessesPermission WRITE setStopAllProcessesPermission NOTIFY stopAllProcessesPermissionChanged)
public:
    explicit ShutdownService(QObject *parent = nullptr);
    ~ShutdownService() override;

    /**
     * Shutdown the system, requires permission @link{shutdownSystemPermission}
     **/
    Q_INVOKABLE void shutdownSystem();

    /**
     * Restarts the system, requires permission @link{restartSystemPermission}.
     **/
    Q_INVOKABLE void restartSystem();

    /**
     * Stops all processes, requires permission @link{stopAllProcessesPermission}.
     *
     * This is mostly development option.
     **/
    Q_INVOKABLE void stopAllProcesses();

    int shutdownSystemPermission() const
    {
        return m_shutdownSystemPermission;
    }
    void setShutdownSystemPermission(int permission);

    int restartSystemPermission() const
    {
        return m_restartSystemPermission;
    }
    void setRestartSystemPermission(int permission);

    int stopAllProcessesPermission() const
    {
        return m_stopAllProcessesPermission;
    }
    void setStopAllProcessesPermission(int permission);

Q_SIGNALS:
    void shutdownSystemPermissionChanged();
    void restartSystemPermissionChanged();
    void stopAllProcessesPermissionChanged();

private:
    /**
     * The stations this ShutdownService interacts with.
     **/
    enum class Station {
        Hardware,
        Simulation
    };

    /**
     * The requests this ShutdownService can send to the stations.
     **/
    enum class Request {
        StopAll,
        ShutdownSystem,
        RestartSystem
    };

    /**
     * Sends the @p request to the @p station.
     **/
    void sendRequest(Request request, Station station);

    /**
     * Gets the pid for @p station from the ConnectionConfiguration.
     **/
    int getPid(Station station) const;

    /**
     * Gets the name for the @p station, either taken from WM_STATION_NAME env variable
     * or hardcoded name of Simulation station.
     **/
    std::string getName(Station station) const;

    int m_shutdownSystemPermission = -1;
    int m_restartSystemPermission = -1;
    int m_stopAllProcessesPermission = -1;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ShutdownService*)
