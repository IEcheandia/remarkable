#pragma once

#include <QAbstractListModel>

#include "seamSeriesStatistics.h"

namespace precitec
{
namespace storage
{

class ResultsStatisticsController;

class ResultsStatisticsSeamSeriesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::ResultsStatisticsController* resultsStatisticsController READ resultsStatisticsController WRITE setResultsStatisticsController NOTIFY resultsStatisticsControllerChanged)

public:
    explicit ResultsStatisticsSeamSeriesModel(QObject* parent = nullptr);
    ~ResultsStatisticsSeamSeriesModel() override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    precitec::storage::ResultsStatisticsController* resultsStatisticsController() const
    {
        return m_resultsStatisticsController;
    }
    void setResultsStatisticsController(ResultsStatisticsController* controller);

Q_SIGNALS:
    void resultsStatisticsControllerChanged();

private:
    void updateModel();

    std::vector<SeamSeriesStatistics> m_seriesStatistics;

    ResultsStatisticsController* m_resultsStatisticsController = nullptr;
    QMetaObject::Connection m_resultsStatisticsControllerDestroyed;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ResultsStatisticsSeamSeriesModel*)
