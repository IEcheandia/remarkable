#pragma once

#include "resultsStatisticsAbstractErrorModel.h"

namespace precitec
{
namespace storage
{

class ResultsStatisticsSeamSeriesErrorModel : public ResultsStatisticsAbstractErrorModel
{
    Q_OBJECT

public:
    explicit ResultsStatisticsSeamSeriesErrorModel(QObject* parent = nullptr);

private:
    void updateModel() override;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ResultsStatisticsSeamSeriesErrorModel*)

