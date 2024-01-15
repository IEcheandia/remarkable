#include "resultsStatisticsSeamErrorModel.h"
#include "resultsStatisticsController.h"

namespace precitec
{
namespace storage
{

ResultsStatisticsSeamErrorModel::ResultsStatisticsSeamErrorModel(QObject* parent)
    : ResultsStatisticsSeamsErrorModel(parent)
{
    connect(this, &ResultsStatisticsSeamErrorModel::seamIdChanged, this, &ResultsStatisticsSeamErrorModel::updateModel);
    connect(this, &ResultsStatisticsSeamErrorModel::linkedSeamIdChanged, this, &ResultsStatisticsSeamErrorModel::updateModel);
    connect(this, &ResultsStatisticsSeamErrorModel::includeLinkedSeamsChanged, this, &ResultsStatisticsSeamErrorModel::updateModel);
}

void ResultsStatisticsSeamErrorModel::setSeamId(const QUuid&id)
{
    if (m_seamId == id)
    {
        return;
    }
    m_seamId = id;
    emit seamIdChanged();
}

void ResultsStatisticsSeamErrorModel::setLinkedSeamId(const QUuid& id)
{
    if (m_linkedSeamId == id)
    {
        return;
    }
    m_linkedSeamId = id;
    emit linkedSeamIdChanged();
}

void ResultsStatisticsSeamErrorModel::setIncludeLinkedSeams(bool set)
{
    if (m_includeLinked == set)
    {
        return;
    }
    m_includeLinked = set;
    emit includeLinkedSeamsChanged();
}

void ResultsStatisticsSeamErrorModel::updateModel()
{
    beginResetModel();

    clear();

    if (resultsStatisticsController() && !seamSeriesId().isNull() && !m_seamId.isNull())
    {
        const auto& seriesStatistics = resultsStatisticsController()->productStatistics().seriesStatistics();
        const auto& seriesId = seamSeriesId();

        auto series_it = std::find_if(seriesStatistics.begin(), seriesStatistics.end(), [&seriesId] (const auto& seriesStats) { return seriesId == seriesStats.uuid(); });

        if (series_it != seriesStatistics.end())
        {
            const auto& seamStatistics = series_it->seamStatistics();

            auto seam_it = std::find_if(seamStatistics.begin(), seamStatistics.end(), [this] (const auto& seamStats) { return m_seamId == seamStats.uuid(); });

            if (seam_it != seamStatistics.end())
            {
                if (m_linkedSeamId.isNull())
                {
                    if (m_includeLinked)
                    {
                        setNios(seam_it->linkedNios());
                    } else
                    {
                        setNios(seam_it->nios());
                    }
                } else
                {
                    const auto& linkedSeamStatistics = seam_it->linkedSeams();

                    auto linkedSeam_it = std::find_if(linkedSeamStatistics.begin(), linkedSeamStatistics.end(), [this] (const auto& linkedSeamStats) { return m_linkedSeamId == linkedSeamStats.uuid(); });

                    if (linkedSeam_it != linkedSeamStatistics.end())
                    {
                        setNios(linkedSeam_it->nios());
                    }
                }
            }
        }
    }

    endResetModel();
}

}
}
