#include "latestProductErrorsModel.h"
#include "resultHelper.h"
#include "resultsServer.h"
#include "seam.h"
#include "errorSettingModel.h"

#include <QTimer>
#include <QColor>

using precitec::storage::ResultsServer;
using precitec::storage::Seam;
using precitec::storage::ErrorSettingModel;
using precitec::interface::ResultArgs;
using precitec::interface::ResultType;

namespace precitec
{
namespace gui
{

LatestProductErrorsModel::LatestProductErrorsModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_queueTimer(new QTimer{this})
{
    m_queueTimer->setInterval(std::chrono::milliseconds{100});
    connect(m_queueTimer, &QTimer::timeout, this, &LatestProductErrorsModel::update);
    connect(this, &LatestProductErrorsModel::liveUpdateChanged, this,
        [this]
        {
            if (m_liveUpdate)
            {
                update();
                m_queueTimer->start();
            } else
            {
                m_queueTimer->stop();
            }
        });
}

LatestProductErrorsModel::~LatestProductErrorsModel() = default;

int LatestProductErrorsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_errors.size();
}

QHash<int, QByteArray> LatestProductErrorsModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("seam")},
        {Qt::UserRole, QByteArrayLiteral("name")},
        {Qt::UserRole + 1, QByteArrayLiteral("color")},
        {Qt::UserRole + 2, QByteArrayLiteral("position")}
    };
}

QVariant LatestProductErrorsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    if (index.row() < 0 || index.row() >= int(m_errors.size()))
    {
        return {};
    }
    if (role == Qt::DisplayRole)
    {
        return QStringLiteral("Seam ").append(QString::number(std::get<0>(m_errors.at(index.row()))));
    }
    if (role == Qt::UserRole)
    {
        return std::get<QString>(m_errors.at(index.row()));
    }
    if (role == Qt::UserRole + 1)
    {
        return std::get<QColor>(m_errors.at(index.row()));
    }
    if (role == Qt::UserRole + 2)
    {
        return std::get<qreal>(m_errors.at(index.row()));
    }
    return {};
}

void LatestProductErrorsModel::setResultsServer(ResultsServer *server)
{
    if (m_resultsServer == server)
    {
        return;
    }
    disconnect(m_resultsServerDestroyedConnection);
    if (m_resultsServer)
    {
        disconnect(m_resultsServer, &ResultsServer::productInspectionStarted, this, &LatestProductErrorsModel::clear);
        disconnect(m_resultsServer, &ResultsServer::nioReceived, this, &LatestProductErrorsModel::addToQueue);
        disconnect(m_resultsServer, &ResultsServer::seamInspectionStarted, this, &LatestProductErrorsModel::updateSeamNumber);
    }

    m_resultsServer = server;
    if (m_resultsServer)
    {
        m_resultsServerDestroyedConnection = connect(m_resultsServer, &QObject::destroyed, this,
            [this]
            {
                m_resultsServer = nullptr;
                emit resultsServerChanged();
            }
        );
        connect(m_resultsServer, &ResultsServer::productInspectionStarted, this, &LatestProductErrorsModel::clear);
        connect(m_resultsServer, &ResultsServer::nioReceived, this, &LatestProductErrorsModel::addToQueue, Qt::DirectConnection);
        connect(m_resultsServer, &ResultsServer::seamInspectionStarted, this, &LatestProductErrorsModel::updateSeamNumber);
    } else
    {
        m_resultsServerDestroyedConnection = QMetaObject::Connection();
    }
    emit resultsServerChanged();
}

void LatestProductErrorsModel::setErrorConfigModel(ErrorSettingModel *model)
{
    if (m_errorConfigModel == model)
    {
        return;
    }
    m_errorConfigModel = model;
    disconnect(m_errorConfigModelDestroyedConnection);
    disconnect(m_errorConfigResetConnection);
    disconnect(m_errorConfigChangedConnection);
    if (m_errorConfigModel)
    {
        m_errorConfigModelDestroyedConnection = connect(m_errorConfigModel, &QObject::destroyed, this, std::bind(&LatestProductErrorsModel::setErrorConfigModel, this, nullptr));
        m_errorConfigResetConnection = connect(m_errorConfigModel, &QAbstractItemModel::modelReset, this, &LatestProductErrorsModel::updateSettings);
        m_errorConfigChangedConnection = connect(m_errorConfigModel, &QAbstractItemModel::dataChanged, this, &LatestProductErrorsModel::updateSettings);
    } else
    {
        m_errorConfigModelDestroyedConnection = {};
        m_errorConfigResetConnection = {};
        m_errorConfigChangedConnection = {};
    }

    emit errorConfigModelChanged();
}

void LatestProductErrorsModel::setLiveUpdate(bool set)
{
    if (m_liveUpdate == set)
    {
        return;
    }
    m_liveUpdate = set;
    emit liveUpdateChanged();
}

void LatestProductErrorsModel::updateSeamNumber(QPointer<Seam> seam, const QUuid &productInstance, quint32 serialNumber)
{
    Q_UNUSED(productInstance)
    Q_UNUSED(serialNumber)

    if (seam)
    {
        m_currentSeamNumber = seam->visualNumber();
        emit seamNumberChanged();
    }
}

void LatestProductErrorsModel::addToQueue(const ResultArgs &result)
{
    if (!result.isNio())
    {
        return;
    }
    if (m_currentSeamNumber < 0)
    {
        return;
    }
    QMutexLocker lock{&m_queueMutex};
    m_queue.push_back(std::make_pair(m_currentSeamNumber, result));
}

void LatestProductErrorsModel::update()
{
    QMutexLocker lock{&m_queueMutex};
    auto queue = std::move(m_queue);
    m_queue.clear();
    lock.unlock();

    while (!queue.empty())
    {
        const auto result = queue.front();

        const auto resultType = result.second.resultType();
        auto errorSetting = m_errorConfigModel ? m_errorConfigModel->getItem(resultType) : nullptr;

        beginInsertRows(QModelIndex(), m_errors.size(), m_errors.size());
        const auto position = result.second.context().position() / 1000.0;
        if (errorSetting)
        {
            m_errors.push_back(std::make_tuple(result.first, resultType, errorSetting->name(), QColor(errorSetting->lineColor()), position));
        } else
        {
            m_errors.push_back(std::make_tuple(result.first, resultType, nameForResult(result.second), colorForResult(result.second), position));
        }
        endInsertRows();

        queue.pop_front();
    }
}


void LatestProductErrorsModel::updateSettings()
{
    auto idx = 0;
    for (auto &it : m_errors)
    {
        QVector<int> changes;
        auto addChange = [&changes] (int change)
        {
            if (changes.contains(change))
            {
                return;
            }
            changes.append(change);
        };

        auto errrConfig = m_errorConfigModel ? m_errorConfigModel->getItem(ResultType(std::get<1>(it))) : nullptr;
        if (errrConfig != nullptr)
        {
            if (std::get<QString>(it).compare(errrConfig->name()) != 0)
            {
                std::get<QString>(it) = errrConfig->name();
                addChange(Qt::UserRole);
            }
            if (std::get<QColor>(it) != QColor(errrConfig->lineColor()))
            {
                std::get<QColor>(it) = QColor(errrConfig->lineColor());
                addChange(Qt::UserRole + 1);
            }
        }

        if (changes.isEmpty() || m_errors.empty())
        {
            idx++;
            continue;
        }

        const auto i = index(idx);
        idx++;
        emit dataChanged(i, i, changes);
    }
}

void LatestProductErrorsModel::clear()
{
    beginResetModel();
    m_errors.clear();
    endResetModel();

    QMutexLocker lock{&m_queueMutex};
    m_queue.clear();
}

}
}
