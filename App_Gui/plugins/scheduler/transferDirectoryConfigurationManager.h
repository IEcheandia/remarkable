#pragma once

#include <QObject>
#include <QVariantMap>

#include <optional>

namespace precitec
{
namespace gui
{
namespace components
{
namespace scheduler
{

/**
 * Configuration manager for the transfer directory to remote location scheduler task.
 **/
class TransferDirectoryConfigurationManager : public QObject
{
    Q_OBJECT
    /**
     * The remote ip address
     **/
    Q_PROPERTY(QString ip READ ip WRITE setIp NOTIFY ipChanged)
    /**
     * The remote user name
     **/
    Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
    /**
     * The remote password
     **/
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    /**
     * The path on the remote host
     **/
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)

    /**
     * Optional file name, only used if @link{hasFileName} is @c true
     * @see hasFileName
     **/
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)

    /**
     * Whether the TransferDirectoryConfigurationManager manages a file name.
     * @see fileName
     **/
    Q_PROPERTY(bool hasFileName READ hasFileName WRITE setHasFileName NOTIFY hasFileNameChanged)

    /**
     * Whether log messages should be written to a debug log file.
     **/
    Q_PROPERTY(bool debug READ debug WRITE setDebug NOTIFY debugChanged)

    /**
     * The settings containing the configuration as a key-value map
     */
    Q_PROPERTY(QVariantMap settings READ settings WRITE setSettings NOTIFY settingsChanged)

    /**
     * The remote port. Optional, if not set @c -1 is returned, internally the default port of the @link protocol is used.
     **/
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)

    /**
     * The protocol for connecting with the remote system
     **/
    Q_PROPERTY(precitec::gui::components::scheduler::TransferDirectoryConfigurationManager::Protocol protocol READ protocol WRITE setProtocol NOTIFY protocolChanged)

    /**
     * The http method. Only used/persisted if @link protocol is either @link{Protocol::Http} or @link{Protocol::Https}. By default set to POST.
     **/
    Q_PROPERTY(precitec::gui::components::scheduler::TransferDirectoryConfigurationManager::HttpMethod httpMethod READ httpMethod WRITE setHttpMethod NOTIFY httpMethodChanged)
public:
    TransferDirectoryConfigurationManager(QObject *parent = nullptr);
    ~TransferDirectoryConfigurationManager() override;

    const QString &ip() const
    {
        return m_ip;
    }
    void setIp(const QString &ip);

    const QString &userName() const
    {
        return m_userName;
    }
    void setUserName(const QString &userName);

    const QString &password() const
    {
        return m_password;
    }
    void setPassword(const QString &password);

    const QString &path() const
    {
        return m_path;
    }
    void setPath(const QString &path);

    const QString &fileName() const
    {
        return m_fileName;
    }
    void setFileName(const QString &fileName);

    bool hasFileName() const
    {
        return m_hasFileName;
    }
    void setHasFileName(bool set);

    bool debug() const
    {
        return m_debug;
    }
    void setDebug(bool set);

    QVariantMap settings() const
    {
        return m_settings;
    }
    void setSettings(const QVariantMap &settings);

    Q_INVOKABLE QVariantMap save() const;

    int port() const
    {
        return m_port.value_or(-1);
    }
    void setPort(int port);

    enum class Protocol
    {
        Sftp,
        Http,
        Https,
    };
    Q_ENUM(Protocol)

    Protocol protocol() const
    {
        return m_protocol;
    }
    void setProtocol(Protocol protocol);

    /**
     * @returns All supported protocols, as human readable stringlist.
     **/
    Q_INVOKABLE QStringList protocols() const;
    /**
     * @returns The default port of the given @p protocol.
     **/
    Q_INVOKABLE int defaultPort(precitec::gui::components::scheduler::TransferDirectoryConfigurationManager::Protocol protocol) const;

    enum class HttpMethod
    {
        Post,
        Put,
    };
    Q_ENUM(HttpMethod);
    HttpMethod httpMethod() const
    {
        return m_httpMethod;
    }
    void setHttpMethod(HttpMethod method);

    /**
     * @returns All supported http methods as human readable stringlist.
     **/
    Q_INVOKABLE QStringList httpMethods() const;

Q_SIGNALS:
    void pathChanged();
    void ipChanged();
    void userNameChanged();
    void passwordChanged();
    void fileNameChanged();
    void hasFileNameChanged();
    void debugChanged();
    void settingsChanged();
    void portChanged();
    void protocolChanged();
    void httpMethodChanged();

private:
    void init();
    QString m_path;
    QString m_ip;
    QString m_userName;
    QString m_password;
    QString m_fileName;
    bool m_hasFileName = false;
    bool m_debug = false;
    std::optional<int> m_port;
    Protocol m_protocol{Protocol::Sftp};
    HttpMethod m_httpMethod{HttpMethod::Post};

    QVariantMap m_settings;
};

}
}
}
}
