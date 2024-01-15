#pragma once

#include "resultsStatisticsAbstractErrorModel.h"

#include <QUuid>

namespace precitec
{
namespace storage
{

class ResultsStatisticsSeamsErrorModel : public ResultsStatisticsAbstractErrorModel
{
    Q_OBJECT

    Q_PROPERTY(QUuid seamSeriesId READ seamSeriesId WRITE setSeamSeriesId NOTIFY seamSeriesIdChanged)

public:
    explicit ResultsStatisticsSeamsErrorModel(QObject* parent = nullptr);

    QUuid seamSeriesId() const
    {
        return m_seamSeriesId;
    }
    void setSeamSeriesId(const QUuid &id);

Q_SIGNALS:
    void seamSeriesIdChanged();

private:
    void updateModel() override;

    QUuid m_seamSeriesId;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ResultsStatisticsSeamsErrorModel*)
