#include "transferDirectoryConfigurationManager.h"


namespace precitec
{
namespace gui
{
namespace components
{
namespace scheduler
{
static const QLatin1String s_sftp{"sftp"};
static const QLatin1String s_http{"http"};
static const QLatin1String s_https{"https"};
static const QLatin1String s_post{"POST"};
static const QLatin1String s_put{"PUT"};

TransferDirectoryConfigurationManager::TransferDirectoryConfigurationManager(QObject* parent)
    : QObject(parent)
{
    connect(this, &TransferDirectoryConfigurationManager::settingsChanged, this, &TransferDirectoryConfigurationManager::init);
}

TransferDirectoryConfigurationManager::~TransferDirectoryConfigurationManager() = default;

void TransferDirectoryConfigurationManager::setSettings(const QVariantMap& settings)
{
    if (m_settings == settings)
    {
        return;
    }
    m_settings = settings;
    emit settingsChanged();
}

void TransferDirectoryConfigurationManager::setIp(const QString& ip)
{
    if (m_ip == ip)
    {
        return;
    }
    m_ip = ip;
    emit ipChanged();
}

void TransferDirectoryConfigurationManager::setPassword(const QString& password)
{
    if (m_password == password)
    {
        return;
    }
    m_password = password;
    emit passwordChanged();
}

void TransferDirectoryConfigurationManager::setUserName(const QString& userName)
{
    if (m_userName == userName)
    {
        return;
    }
    m_userName = userName;
    emit userNameChanged();
}

void TransferDirectoryConfigurationManager::setPath(const QString& path)
{
    if (m_path == path)
    {
        return;
    }
    m_path = path;
    emit pathChanged();
}

void TransferDirectoryConfigurationManager::setFileName(const QString& fileName)
{
    if (m_fileName == fileName)
    {
        return;
    }
    m_fileName = fileName;
    emit fileNameChanged();
}

void TransferDirectoryConfigurationManager::setHasFileName(bool set)
{
    if (m_hasFileName == set)
    {
        return;
    }
    m_hasFileName = set;
    emit hasFileNameChanged();
}

void TransferDirectoryConfigurationManager::setDebug(bool set)
{
    if (m_debug == set)
    {
        return;
    }
    m_debug = set;
    emit debugChanged();
}

void TransferDirectoryConfigurationManager::setPort(int port)
{
    if (m_port == port)
    {
        return;
    }
    if (port == -1)
    {
        m_port.reset();
    }
    else
    {
        m_port = port;
    }
    emit portChanged();
}

void TransferDirectoryConfigurationManager::init()
{
    if (auto it = m_settings.find(QStringLiteral("TargetIpAddress")); it != m_settings.end())
    {
        setIp(it.value().toString());
    }

    if (auto it = m_settings.find(QStringLiteral("TargetUserName")); it != m_settings.end())
    {
        setUserName(it.value().toString());
    }

    if (auto it = m_settings.find(QStringLiteral("TargetPassword")); it != m_settings.end())
    {
        setPassword(it.value().toString());
    }

    if (auto it = m_settings.find(QStringLiteral("TargetDirectoryPath")); it != m_settings.end())
    {
        setPath(it.value().toString());
    }

    if (auto it = m_settings.find(QStringLiteral("TargetFileName")); it != m_settings.end())
    {
        setFileName(it.value().toString());
    }
    if (auto it = m_settings.find(QStringLiteral("DebugOptionStatus")); it != m_settings.end())
    {
        setDebug(it.value().toBool());
    }
    if (auto it = m_settings.find(QStringLiteral("Port")); it != m_settings.end())
    {
        setPort(it.value().toInt());
    }
    if (auto it = m_settings.find(QStringLiteral("Protocol")); it != m_settings.end())
    {
        const auto protocol = it.value().toString().toLower();
        if (protocol == s_sftp)
        {
            setProtocol(Protocol::Sftp);
        }
        else if (protocol == s_http)
        {
            setProtocol(Protocol::Http);
        }
        else if (protocol == s_https)
        {
            setProtocol(Protocol::Https);
        }
    }
    if (auto it = m_settings.find(QStringLiteral("HttpMethod")); it != m_settings.end())
    {
        const auto method = it.value().toString().toUpper();
        if (method == s_post)
        {
            setHttpMethod(HttpMethod::Post);
        }
        else if (method == s_put)
        {
            setHttpMethod(HttpMethod::Put);
        }
    }
}

void TransferDirectoryConfigurationManager::setProtocol(Protocol protocol)
{
    if (m_protocol == protocol)
    {
        return;
    }
    m_protocol = protocol;
    emit protocolChanged();
}

namespace
{
static QLatin1String toString(TransferDirectoryConfigurationManager::Protocol protocol)
{
    switch (protocol)
    {
    case TransferDirectoryConfigurationManager::Protocol::Sftp:
        return s_sftp;
    case TransferDirectoryConfigurationManager::Protocol::Http:
        return s_http;
    case TransferDirectoryConfigurationManager::Protocol::Https:
        return s_https;
    default:
        __builtin_unreachable();
    }
}

static QLatin1String toString(TransferDirectoryConfigurationManager::HttpMethod method)
{
    switch (method)
    {
    case TransferDirectoryConfigurationManager::HttpMethod::Post:
        return s_post;
    case TransferDirectoryConfigurationManager::HttpMethod::Put:
        return s_put;
    default:
        __builtin_unreachable();
    }
}

}

QVariantMap TransferDirectoryConfigurationManager::save() const
{
    QVariantMap map{
        {QStringLiteral("TargetIpAddress"), ip()},
        {QStringLiteral("TargetUserName"), userName()},
        {QStringLiteral("TargetPassword"), password()},
        {QStringLiteral("TargetDirectoryPath"), path()},
        {QStringLiteral("DebugOptionStatus"), m_debug ? QStringLiteral("true") : QStringLiteral("false")},
        {QStringLiteral("Protocol"), toString(m_protocol)},
    };

    if (hasFileName() && !m_fileName.isEmpty())
    {
        map.insert(QStringLiteral("TargetFileName"), m_fileName);
    }
    if (m_port.has_value())
    {
        map.insert(QStringLiteral("Port"), QString::number(m_port.value()));
    }
    if (m_protocol == Protocol::Http || m_protocol == Protocol::Https)
    {
        map.insert(QStringLiteral("HttpMethod"), toString(m_httpMethod));
    }

    return map;
}

QStringList TransferDirectoryConfigurationManager::protocols() const
{
    return QStringList{
        s_sftp,
        s_http,
        s_https,
    };
}

int TransferDirectoryConfigurationManager::defaultPort(precitec::gui::components::scheduler::TransferDirectoryConfigurationManager::Protocol protocol) const
{
    switch (protocol)
    {
    case Protocol::Sftp:
        return 22;
    case Protocol::Http:
        return 80;
    case Protocol::Https:
        return 443;
    default:
        __builtin_unreachable();
    }
}

void TransferDirectoryConfigurationManager::setHttpMethod(HttpMethod method)
{
    if (m_httpMethod == method)
    {
        return;
    }
    m_httpMethod = method;
    emit httpMethodChanged();
}

QStringList TransferDirectoryConfigurationManager::httpMethods() const
{
    return QStringList{
        s_post,
        s_put,
    };
}

}
}
}
}
