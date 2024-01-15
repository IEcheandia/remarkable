#include "resultsStatisticsSeamsModel.h"
#include "resultsStatisticsController.h"
#include "product.h"
#include "linkedSeam.h"
#include "seam.h"
#include "seamSeries.h"

namespace precitec
{
namespace storage
{

ResultsStatisticsSeamsModel::ResultsStatisticsSeamsModel(QObject* parent)
: QAbstractListModel(parent)
{
    connect(this, &ResultsStatisticsSeamsModel::seamSeriesIdChanged, this, &ResultsStatisticsSeamsModel::updateModel);
    connect(this, &ResultsStatisticsSeamsModel::resultsStatisticsControllerChanged, this, &ResultsStatisticsSeamsModel::updateModel);
}

ResultsStatisticsSeamsModel::~ResultsStatisticsSeamsModel() = default;

QVariant ResultsStatisticsSeamsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= int(m_seamStatistics.size()))
    {
        return {};
    }

    const auto& seamStats = m_seamStatistics.at(index.row());

    switch(role)
    {
        case Qt::DisplayRole:
            return seamStats.name();
        case Qt::UserRole:
            return seamStats.uuid();
        case Qt::UserRole + 1:
            return seamStats.ioInPercent();
        case Qt::UserRole + 2:
            return seamStats.nioInPercent();
        case Qt::UserRole + 3:
            return seamStats.ioCount();
        case Qt::UserRole + 4:
            return seamStats.nioCount();
        case Qt::UserRole + 5:
            return seamStats.visualNumber();
        case Qt::UserRole + 6:
            return seamStats.includesLinkedSeams();
        case Qt::UserRole + 7:
            return seamStats.ioCountIncludingLinkedSeamsInPercent();
        case Qt::UserRole + 8:
            return seamStats.nioCountIncludingLinkedSeamsInPercent();
        case Qt::UserRole + 9:
            return seamStats.ioCountIncludingLinkedSeams();
        case Qt::UserRole + 10:
            return seamStats.nioCountIncludingLinkedSeams();
    }
    return {};
}

int ResultsStatisticsSeamsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_seamStatistics.size();
}

QHash<int, QByteArray> ResultsStatisticsSeamsModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("id")},
        {Qt::UserRole + 1, QByteArrayLiteral("ioInPercent")},
        {Qt::UserRole + 2, QByteArrayLiteral("nioInPercent")},
        {Qt::UserRole + 3, QByteArrayLiteral("io")},
        {Qt::UserRole + 4, QByteArrayLiteral("nio")},
        {Qt::UserRole + 5, QByteArrayLiteral("visualNumber")},
        {Qt::UserRole + 6, QByteArrayLiteral("includesLinkedSeams")},
        {Qt::UserRole + 7, QByteArrayLiteral("ioInPercentIncludingLinkedSeams")},
        {Qt::UserRole + 8, QByteArrayLiteral("nioInPercentIncludingLinkedSeams")},
        {Qt::UserRole + 9, QByteArrayLiteral("ioIncludingLinkedSeams")},
        {Qt::UserRole + 10, QByteArrayLiteral("nioIncludingLinkedSeams")},
    };
}

void ResultsStatisticsSeamsModel::setResultsStatisticsController(precitec::storage::ResultsStatisticsController* controller)
{
    if (m_resultsStatisticsController == controller)
    {
        return;
    }

    if (m_resultsStatisticsController)
    {
        disconnect(m_resultsStatisticsControllerDestroyed);
        disconnect(m_resultsStatisticsController, &ResultsStatisticsController::update, this, &ResultsStatisticsSeamsModel::updateModel);
    }

    m_resultsStatisticsController = controller;

    if (m_resultsStatisticsController)
    {
        m_resultsStatisticsControllerDestroyed = connect(m_resultsStatisticsController, &ResultsStatisticsController::destroyed, this, std::bind(&ResultsStatisticsSeamsModel::setResultsStatisticsController, this, nullptr));
        connect(m_resultsStatisticsController, &ResultsStatisticsController::update, this, &ResultsStatisticsSeamsModel::updateModel);
    } else
    {
        m_resultsStatisticsControllerDestroyed = {};
    }

    emit resultsStatisticsControllerChanged();
}

void ResultsStatisticsSeamsModel::setSeamSeriesId(const QUuid &id)
{
    if (m_seamSeriesId == id)
    {
        return;
    }
    m_seamSeriesId = id;
    emit seamSeriesIdChanged();
}

void ResultsStatisticsSeamsModel::updateModel()
{
    beginResetModel();

    m_seamStatistics.clear();

    if (m_resultsStatisticsController && !m_seamSeriesId.isNull())
    {
        const auto& seriesStatistics = m_resultsStatisticsController->productStatistics().seriesStatistics();

        auto it = std::find_if(seriesStatistics.begin(), seriesStatistics.end(), [this] (const auto& seriesStats) { return m_seamSeriesId == seriesStats.uuid(); });

        if (it != seriesStatistics.end())
        {
            m_seamStatistics = it->seamStatistics();
        }
    }

    endResetModel();
}

}
}


