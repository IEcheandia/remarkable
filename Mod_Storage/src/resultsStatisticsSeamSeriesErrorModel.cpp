#include "resultsStatisticsSeamSeriesErrorModel.h"
#include "resultsStatisticsController.h"

namespace precitec
{
namespace storage
{

ResultsStatisticsSeamSeriesErrorModel::ResultsStatisticsSeamSeriesErrorModel(QObject* parent)
    : ResultsStatisticsAbstractErrorModel(parent)
{
}

void ResultsStatisticsSeamSeriesErrorModel::updateModel()
{
    beginResetModel();

    clear();

    if (resultsStatisticsController())
    {
        setNios(resultsStatisticsController()->productStatistics().nios());
    }

    endResetModel();
}

}
}
