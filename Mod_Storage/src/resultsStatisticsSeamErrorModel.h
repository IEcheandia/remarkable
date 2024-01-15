#pragma once

#include "resultsStatisticsSeamsErrorModel.h"

#include <QUuid>

namespace precitec
{
namespace storage
{

class ResultsStatisticsSeamErrorModel : public ResultsStatisticsSeamsErrorModel
{
    Q_OBJECT

    Q_PROPERTY(QUuid seamId READ seamId WRITE setSeamId NOTIFY seamIdChanged)

    Q_PROPERTY(QUuid linkedSeamId READ linkedSeamId WRITE setLinkedSeamId NOTIFY linkedSeamIdChanged)

    Q_PROPERTY(bool includeLinkedSeams READ includeLinkedSeams WRITE setIncludeLinkedSeams NOTIFY includeLinkedSeamsChanged)

public:
    explicit ResultsStatisticsSeamErrorModel(QObject* parent = nullptr);

    const QUuid& seamId() const
    {
        return m_seamId;
    }
    void setSeamId(const QUuid& id);

    const QUuid& linkedSeamId() const
    {
        return m_seamId;
    }
    void setLinkedSeamId(const QUuid& id);

    bool includeLinkedSeams() const
    {
        return m_includeLinked;
    }
    void setIncludeLinkedSeams(bool set);

Q_SIGNALS:
    void seamIdChanged();
    void linkedSeamIdChanged();
    void includeLinkedSeamsChanged();

private:
    void updateModel() override;

    QUuid m_seamId;
    QUuid m_linkedSeamId;
    bool m_includeLinked = false;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ResultsStatisticsSeamErrorModel*)
