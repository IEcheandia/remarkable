#include "logModel.h"
#include "logReceiver.h"
#include "moduleModel.h"

#include "../general/languageSupport.h"

#include <precitec/userManagement.h>

#include <QDateTime>
#include <QThread>

using precitec::gui::components::user::UserManagement;

namespace precitec
{
namespace gui
{
namespace components
{
namespace logging
{

LogModel::LogModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_moduleModel(new ModuleModel(this))
{
    m_moduleModel->setLogModel(this);
    connect(UserManagement::instance(), &UserManagement::currentUserChanged, this, &LogModel::canClearChanged);
    connect(this, &LogModel::pausedChanged, this,
        [this]
        {
            if (m_paused)
            {
                return;
            }
            addItems(std::move(m_pauseQueue));
            m_pauseQueue = {};
        });
    connect(this, &LogModel::stationChanged, this, &LogModel::initReceiverThread);
}

LogModel::~LogModel()
{
    if (m_receiver)
    {
        m_receiver->deleteLater();
    }
    if (m_receiverThread)
    {
        m_receiverThread->quit();
        m_receiverThread->wait();
    }
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    const auto &item = m_logItems.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return format(item);
    case Qt::UserRole:
    {
        const std::chrono::microseconds time(item.timestamp().epochMicroseconds());
        return QDateTime::fromMSecsSinceEpoch(std::chrono::duration_cast<std::chrono::milliseconds>(time).count());
    }
    case Qt::UserRole+1:
        return QByteArray::fromStdString(item.moduleName());
    case Qt::UserRole+2:
        return QVariant::fromValue(LogLevel(item.type()));
    default:
        return QVariant();
    }
}

namespace
{

/**
 * Seeks to the next index of a "%" not followed by a "%".
 * Replaces all "%%" on the fly with "%"
 **/
int findNextMarkerReplacingEscape(QString &message, int startIndex)
{
    int index = message.indexOf(QLatin1Char('%'), startIndex);
    if (index == -1)
    {
        return -1;
    }
    if (index + 1 == message.length())
    {
        return -1;
    }
    if (message.at(index + 1) == QLatin1Char('%'))
    {
        message.replace(index, 2, QLatin1String("%"));
        return findNextMarkerReplacingEscape(message, index + 1);
    }
    return index;
}
}

QString LogModel::format(const precitec::interface::wmLogItem &item) const
{
    QString message = QString::fromStdString(item.message()).trimmed();
    if (!item.key().empty())
    {
        const auto key = QString::fromStdString(item.key());
        const auto translated = LanguageSupport::instance()->getString(key);
        // in case the key is not translated, the key is returned and then we use the untranslated message
        if (translated != key)
        {
            message = translated.trimmed();
        }
    }
    int index = 0;
    for (const auto &param : item.getParams())
    {
        index = findNextMarkerReplacingEscape(message, index);
        const int nextCharIndex = index + 1;
        if (index == -1)
        {
            break;
        }
        const auto &character = message.at(nextCharIndex);
        if (character == QLatin1Char('d') || character == QLatin1Char('i'))
        {
            message.replace(index, 2, QString::number(int(param.value())));
        } else if (character == QLatin1Char('u'))
        {
            message.replace(index, 2, QString::number(uint(param.value())));
        } else if (character == QLatin1Char('x'))
        {
            message.replace(index, 2, QString::number(uint(param.value()), 16));
        } else if (character == QLatin1Char('f'))
        {
            message.replace(index, 2, QString::number(param.value()));
        } else if (character == QLatin1Char('s'))
        {
            const QString replaceString = QString::fromStdString(param.string());
            message.replace(index, 2, replaceString);
            index += replaceString.length();
        }
    }
    findNextMarkerReplacingEscape(message, index);
    return message;
}

int LogModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_logItems.size();
}

QHash<int, QByteArray> LogModel::roleNames() const
{
    return QHash<int, QByteArray>{
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::UserRole, QByteArrayLiteral("dateTime")},
        {Qt::UserRole+1, QByteArrayLiteral("module")},
        {Qt::UserRole+2, QByteArrayLiteral("level")}
    };
}

void LogModel::addItems(std::deque<precitec::interface::wmLogItem> &&items)
{
    if (items.size() == 0)
    {
        return;
    }
    for (auto it = items.end(); it != items.begin(); it--)
    {
        const auto type = LogLevel((it - 1)->type());
        if (type == LogLevel::Warning || type == LogLevel::Error || type == LogLevel::Fatal)
        {
            m_latestWarningOrError = format(*(it - 1));
            emit latestWarningOrErrorChanged();
            break;
        }
    }
    if (m_paused)
    {
        if (m_pauseQueue.size() + items.size() >= m_maximumLogMessage)
        {
            if (items.size() >= m_maximumLogMessage)
            {
                m_pauseQueue = std::move(items);
                return;
            } else
            {
                auto end = m_pauseQueue.begin();
                std::advance(end, m_pauseQueue.size() + items.size() - m_maximumLogMessage);
                m_pauseQueue.erase(m_pauseQueue.begin(), end);
            }
        }
        std::move(items.begin(), items.end(), std::back_inserter(m_pauseQueue));
        return;
    }
    if (m_logItems.size() + items.size() >= m_maximumLogMessage)
    {
        if (items.size() >= m_maximumLogMessage)
        {
            beginResetModel();
            m_logItems = std::move(items);
            endResetModel();
            return;
        } else
        {
            // erase elements
            beginRemoveRows(QModelIndex(), 0, m_logItems.size() + items.size() - m_maximumLogMessage - 1);
            auto end = m_logItems.begin();
            std::advance(end, m_logItems.size() + items.size() - m_maximumLogMessage);
            m_logItems.erase(m_logItems.begin(), end);
            endRemoveRows();
        }
    }
    beginInsertRows(QModelIndex(), m_logItems.size(), m_logItems.size() + items.size() - 1);
    std::move(items.begin(), items.end(), std::back_inserter(m_logItems));
    endInsertRows();
}

void LogModel::clear()
{
    if (!canClear())
    {
        return;
    }
    if (!m_latestWarningOrError.isEmpty())
    {
        m_latestWarningOrError = QString{};
        emit latestWarningOrErrorChanged();
    }
    beginResetModel();
    m_logItems.clear();
    m_pauseQueue.clear();
    endResetModel();
}


void LogModel::initReceiverThread()
{
    if (!m_receiverThread)
    {
        m_receiverThread = new QThread(this);
    }
    if (m_receiver)
    {
        m_receiver->deleteLater();
    }
    m_receiver = new LogReceiver;
    m_receiver->setStationName(station());
    m_receiver->moveToThread(m_receiverThread);
    connect(m_receiver, &LogReceiver::logsReceived, this,
        [this]
        {
            addItems(std::forward<std::deque<precitec::interface::wmLogItem>>(m_receiver->takeLogs()));
        }, Qt::QueuedConnection);

    m_receiverThread->start();
    m_receiver->start();
}

QAbstractItemModel *LogModel::moduleModel() const
{
    return m_moduleModel;
}

bool LogModel::canClear() const
{
    if (!UserManagement::instance()->currentUser())
    {
        return true;
    }
    return UserManagement::instance()->hasPermission(m_clearLogMessagesPermission);
}

void LogModel::setPaused(bool paused)
{
    if (m_paused == paused)
    {
        return;
    }
    m_paused = paused;
    emit pausedChanged();
}

void LogModel::setClearLogMessagesPermission(int permission)
{
    if (m_clearLogMessagesPermission == permission)
    {
        return;
    }
    m_clearLogMessagesPermission = permission;
    emit clearLogMessagesPermissionChanged();
}

void LogModel::setStation(const QByteArray &station)
{
    if (station == m_stationName)
    {
        return;
    }
    m_stationName = station;
    emit stationChanged();
}

}
}
}
}
