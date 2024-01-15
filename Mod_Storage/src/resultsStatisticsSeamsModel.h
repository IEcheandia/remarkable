#pragma once

#include <QAbstractListModel>
#include <QUuid>

#include "seamStatistics.h"

namespace precitec
{
namespace storage
{

class ResultsStatisticsController;

class ResultsStatisticsSeamsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::ResultsStatisticsController* resultsStatisticsController READ resultsStatisticsController WRITE setResultsStatisticsController NOTIFY resultsStatisticsControllerChanged)

    Q_PROPERTY(QUuid seamSeriesId READ seamSeriesId WRITE setSeamSeriesId NOTIFY seamSeriesIdChanged)

public:
    explicit ResultsStatisticsSeamsModel(QObject* parent = nullptr);
    ~ResultsStatisticsSeamsModel() override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    precitec::storage::ResultsStatisticsController* resultsStatisticsController() const
    {
        return m_resultsStatisticsController;
    }
    void setResultsStatisticsController(ResultsStatisticsController* controller);

    const QUuid &seamSeriesId() const
    {
        return m_seamSeriesId;
    }
    void setSeamSeriesId(const QUuid & id);

Q_SIGNALS:
    void resultsStatisticsControllerChanged();
    void seamSeriesIdChanged();

private:
    void updateModel();

    QUuid m_seamSeriesId;
    std::vector<SeamStatistics> m_seamStatistics;

    ResultsStatisticsController* m_resultsStatisticsController = nullptr;
    QMetaObject::Connection m_resultsStatisticsControllerDestroyed;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ResultsStatisticsSeamsModel*)
