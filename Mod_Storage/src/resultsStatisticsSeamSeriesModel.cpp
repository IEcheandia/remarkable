#include "resultsStatisticsSeamSeriesModel.h"
#include "resultsStatisticsController.h"
#include "productStatistics.h"

namespace precitec
{
namespace storage
{

ResultsStatisticsSeamSeriesModel::ResultsStatisticsSeamSeriesModel(QObject* parent)
    : QAbstractListModel(parent)
{
    connect(this, &ResultsStatisticsSeamSeriesModel::resultsStatisticsControllerChanged, this, &ResultsStatisticsSeamSeriesModel::updateModel);
}

ResultsStatisticsSeamSeriesModel::~ResultsStatisticsSeamSeriesModel() = default;

QVariant ResultsStatisticsSeamSeriesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= int(m_seriesStatistics.size()))
    {
        return {};
    }

    const auto& seriesStats = m_seriesStatistics.at(index.row());

    switch(role)
    {
        case Qt::DisplayRole:
            return seriesStats.name();
        case Qt::UserRole:
            return seriesStats.uuid();
        case Qt::UserRole + 1:
            return seriesStats.ioInPercent();
        case Qt::UserRole + 2:
            return seriesStats.nioInPercent();
        case Qt::UserRole + 3:
            return seriesStats.ioCount();
        case Qt::UserRole + 4:
            return seriesStats.nioCount();
        case Qt::UserRole + 5:
            return seriesStats.visualNumber();
    }
    return {};
}

int ResultsStatisticsSeamSeriesModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_seriesStatistics.size();
}

QHash<int, QByteArray> ResultsStatisticsSeamSeriesModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("id")},
        {Qt::UserRole + 1, QByteArrayLiteral("ioInPercent")},
        {Qt::UserRole + 2, QByteArrayLiteral("nioInPercent")},
        {Qt::UserRole + 3, QByteArrayLiteral("io")},
        {Qt::UserRole + 4, QByteArrayLiteral("nio")},
        {Qt::UserRole + 5, QByteArrayLiteral("visualNumber")}
    };
}

void ResultsStatisticsSeamSeriesModel::setResultsStatisticsController(precitec::storage::ResultsStatisticsController* controller)
{
    if (m_resultsStatisticsController == controller)
    {
        return;
    }

    if (m_resultsStatisticsController)
    {
        disconnect(m_resultsStatisticsControllerDestroyed);
        disconnect(m_resultsStatisticsController, &ResultsStatisticsController::update, this, &ResultsStatisticsSeamSeriesModel::updateModel);
    }

    m_resultsStatisticsController = controller;

    if (m_resultsStatisticsController)
    {
        m_resultsStatisticsControllerDestroyed = connect(m_resultsStatisticsController, &ResultsStatisticsController::destroyed, this, std::bind(&ResultsStatisticsSeamSeriesModel::setResultsStatisticsController, this, nullptr));
        connect(m_resultsStatisticsController, &ResultsStatisticsController::update, this, &ResultsStatisticsSeamSeriesModel::updateModel);
    } else
    {
        m_resultsStatisticsController = {};
    }

    emit resultsStatisticsControllerChanged();
}

void ResultsStatisticsSeamSeriesModel::updateModel()
{
    beginResetModel();

    m_seriesStatistics.clear();

    if (m_resultsStatisticsController)
    {
        m_seriesStatistics = m_resultsStatisticsController->productStatistics().seriesStatistics();
    }

    endResetModel();
}

}
}

