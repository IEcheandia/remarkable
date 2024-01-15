#include "productInstanceSeriesModel.h"
#include "seamSeriesMetaData.h"

#include <QDir>
#include <QFutureWatcher>
#include <QtConcurrentRun>

namespace precitec
{
namespace storage
{

ProductInstanceSeriesModel::ProductInstanceSeriesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &ProductInstanceSeriesModel::productInstanceChanged, this, &ProductInstanceSeriesModel::update);
}

ProductInstanceSeriesModel::~ProductInstanceSeriesModel() = default;

int ProductInstanceSeriesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_series.size();
}

QVariant ProductInstanceSeriesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant{};
    }
    const auto &seriesData = m_series.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return seriesData.number;
    case Qt::UserRole:
        return QVariant::fromValue(seriesData.nio);
    case Qt::UserRole + 1:
        return seriesData.nioResultsSwitchedOff;
    case Qt::UserRole + 2:
        return seriesData.number + 1;
    }
    return QVariant{};
}

QHash<int, QByteArray> ProductInstanceSeriesModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("number")},
        {Qt::UserRole, QByteArrayLiteral("nio")},
        {Qt::UserRole + 1, QByteArrayLiteral("nioResultsSwitchedOff")},
        {Qt::UserRole + 2, QByteArrayLiteral("visualNumber")}
    };
}

void ProductInstanceSeriesModel::setProductInstance(const QFileInfo &info)
{
    if (info == m_productInstance)
    {
        return;
    }
    m_productInstance = info;
    emit productInstanceChanged();
}

void ProductInstanceSeriesModel::update()
{
    beginResetModel();
    m_series.clear();
    if (m_productInstance.exists() && m_productInstance.isDir())
    {
        QDir dir{m_productInstance.absoluteFilePath()};
        dir.setNameFilters({QStringLiteral("seam_series*")});
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
            loadMetaData(seamSeries, seamSeriesNumber);
            m_series.emplace_back(SeriesData{seamSeriesNumber});
        }
    }

    endResetModel();
}

void ProductInstanceSeriesModel::loadMetaData(const QFileInfo &dir, int seamSeries)
{
    QFutureWatcher<SeamSeriesMetaData> *watcher = new QFutureWatcher<SeamSeriesMetaData>(this);
    connect(watcher, &QFutureWatcher<SeamSeriesMetaData>::finished, this,
        [this, seamSeries, watcher]
        {
            watcher->deleteLater();
            auto it = std::find_if(m_series.begin(), m_series.end(), [seamSeries] (const auto &s) { return s.number == seamSeries; });
            if (it == m_series.end())
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
            if (!changedRoles.empty())
            {
                const QModelIndex idx = index(std::distance(m_series.begin(), it), 0);
                emit dataChanged(idx, idx, changedRoles);
            }
        }
    );
    watcher->setFuture(QtConcurrent::run([dir] { return SeamSeriesMetaData::parse(dir); }));
}

}
}

