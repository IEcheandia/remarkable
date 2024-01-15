#include "productInstanceSeamModel.h"
#include "seamMetaData.h"
#include "seamSeries.h"

#include <QDir>
#include <QFutureWatcher>
#include <QtConcurrentRun>

namespace precitec
{
namespace storage
{

ProductInstanceSeamModel::ProductInstanceSeamModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &ProductInstanceSeamModel::productInstanceChanged, this, &ProductInstanceSeamModel::update);
    connect(this, &ProductInstanceSeamModel::seamSeriesChanged, this, &ProductInstanceSeamModel::update);
}

ProductInstanceSeamModel::~ProductInstanceSeamModel() = default;

int ProductInstanceSeamModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_seams.size();
}

QVariant ProductInstanceSeamModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant{};
    }
    const auto &seamData = m_seams.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return seamData.number;
    case Qt::UserRole:
        return QVariant::fromValue(seamData.nio);
    case Qt::UserRole + 1:
        return seamData.nioResultsSwitchedOff;
    case Qt::UserRole + 2:
        return seamData.length;
    case Qt::UserRole + 3:
        return seamData.number + 1;
    case Qt::UserRole + 4:
        return seamData.uuid;
    }
    return QVariant{};
}

QHash<int, QByteArray> ProductInstanceSeamModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("number")},
        {Qt::UserRole, QByteArrayLiteral("nio")},
        {Qt::UserRole + 1, QByteArrayLiteral("nioResultsSwitchedOff")},
        {Qt::UserRole + 2, QByteArrayLiteral("length")},
        {Qt::UserRole + 3, QByteArrayLiteral("visualNumber")},
        {Qt::UserRole + 4, QByteArrayLiteral("uuid")}
    };
}

void ProductInstanceSeamModel::setProductInstance(const QFileInfo &info)
{
    if (info == m_productInstance)
    {
        return;
    }
    m_productInstance = info;
    emit productInstanceChanged();
}

void ProductInstanceSeamModel::setSeamSeries(SeamSeries *series)
{
    if (m_seamSeries == series)
    {
        return;
    }
    m_seamSeries = series;
    disconnect(m_seamSeriesDestroyedConnection);
    if (m_seamSeries)
    {
        m_seamSeriesDestroyedConnection = connect(m_seamSeries, &QObject::destroyed, this, std::bind(&ProductInstanceSeamModel::setSeamSeries, this, nullptr));
    } else
    {
        m_seamSeriesDestroyedConnection = {};
    }

    emit seamSeriesChanged();
}

void ProductInstanceSeamModel::update()
{
    beginResetModel();
    m_seams.clear();
    m_metaDataLoadCount = 0;
    if (m_productInstance.exists() && m_productInstance.isDir() && m_seamSeries != nullptr )
    {
        QDir dir{m_productInstance.absoluteFilePath()};
        dir.setNameFilters({QStringLiteral("seam_series%2*").arg(m_seamSeries->number(), 4, 10, QLatin1Char('0'))});
        dir.setFilter(QDir::Dirs);
        const auto seamSeriesDirs = dir.entryInfoList();
        for (const auto &seamSeries : seamSeriesDirs)
        {
            bool ok = false;
            const auto seamSeriesNumber = seamSeries.fileName().right(4).toInt(&ok);
            if (!ok)
            {
                continue;
            }
            QDir dir{seamSeries.absoluteFilePath()};
            dir.setNameFilters({QStringLiteral("seam*")});
            dir.setFilter(QDir::Dirs);
            const auto seams = dir.entryInfoList();
            for (const auto &seam : seams)
            {
                bool ok = false;
                const auto number = seam.fileName().right(4).toInt(&ok);
                if (!ok)
                {
                    continue;
                }
                loadMetaData(seam, seamSeriesNumber, number);
                m_seams.emplace_back(SeamData{seamSeriesNumber, number});
            }
        }
    }

    endResetModel();
}

void ProductInstanceSeamModel::loadMetaData(const QFileInfo &dir, int seamSeries, int seam)
{
    auto *watcher = new QFutureWatcher<SeamMetaData>(this);
    connect(watcher, &QFutureWatcher<SeamMetaData>::finished, this,
        [this, seamSeries, seam, watcher]
        {
            watcher->deleteLater();
            auto it = std::find_if(m_seams.begin(), m_seams.end(), [seamSeries, seam] (const auto &s) { return s.seamSeries == seamSeries && s.number == seam; });
            if (it == m_seams.end())
            {
                return;
            }
            const auto metaData = watcher->result();
            QVector<int> changedRoles;
            if (metaData.isNioValid())
            {
                const auto nio = metaData.nio() ? State::Nio : State::Io;
                if (nio != it->nio)
                {
                    it->nio = nio;
                    changedRoles << Qt::UserRole;
                }
            }
            if (metaData.isNioSwitchedOffValid())
            {
                if (metaData.nioSwitchedOff() != it->nioResultsSwitchedOff)
                {
                    it->nioResultsSwitchedOff = metaData.nioSwitchedOff();
                    changedRoles << Qt::UserRole + 1;
                }
            }
            if (metaData.isLengthValid())
            {
                if (metaData.length() != it->length)
                {
                    it->length = metaData.length();
                    changedRoles << Qt::UserRole + 2;
                }
            }
            if (metaData.isUuidValid())
            {
                if (metaData.uuid() != it->uuid)
                {
                    it->uuid = metaData.uuid();
                    changedRoles << Qt::UserRole + 4;
                }
            }
            if (!changedRoles.empty())
            {
                const QModelIndex idx = index(std::distance(m_seams.begin(), it), 0);
                emit dataChanged(idx, idx, changedRoles);
            }
            m_metaDataLoadCount++;

            if (m_metaDataLoadCount == m_seams.size())
            {
                emit metaDataLoadFinished();
            }
        }
    );
    watcher->setFuture(QtConcurrent::run([dir]
        {
            return SeamMetaData::parse(dir);
        }
    ));
}

}
}
