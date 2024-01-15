#include "resultsStatisticsSeamsErrorModel.h"
#include "resultsStatisticsController.h"

namespace precitec
{
namespace storage
{

ResultsStatisticsSeamsErrorModel::ResultsStatisticsSeamsErrorModel(QObject* parent)
    : ResultsStatisticsAbstractErrorModel(parent)
{
    connect(this, &ResultsStatisticsSeamsErrorModel::seamSeriesIdChanged, this, &ResultsStatisticsSeamsErrorModel::updateModel);
}

void ResultsStatisticsSeamsErrorModel::setSeamSeriesId(const QUuid &id)
{
    if (m_seamSeriesId == id)
    {
        return;
    }
    m_seamSeriesId = id;
    emit seamSeriesIdChanged();
}

void ResultsStatisticsSeamsErrorModel::updateModel()
{
    beginResetModel();

    clear();

    if (resultsStatisticsController() && !m_seamSeriesId.isNull())
    {
        const auto& seriesStatistics = resultsStatisticsController()->productStatistics().seriesStatistics();

        auto it = std::find_if(seriesStatistics.begin(), seriesStatistics.end(), [this] (const auto& seriesStats) { return m_seamSeriesId == seriesStats.uuid(); });

        if (it != seriesStatistics.end())
        {
            setNios(it->nios());
        }
    }

    endResetModel();
}

}
}
