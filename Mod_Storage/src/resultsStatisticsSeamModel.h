#pragma once

#include <QAbstractListModel>
#include <QUuid>

#include "seamStatistics.h"

namespace precitec
{
namespace storage
{

class ResultsStatisticsController;

class ResultsStatisticsSeamModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::ResultsStatisticsController* resultsStatisticsController READ resultsStatisticsController WRITE setResultsStatisticsController NOTIFY resultsStatisticsControllerChanged)

    Q_PROPERTY(QUuid seamSeriesId READ seamSeriesId WRITE setSeamSeriesId NOTIFY seamSeriesIdChanged)

    Q_PROPERTY(QUuid seamId READ seamId WRITE setSeamId NOTIFY seamIdChanged)

    Q_PROPERTY(QUuid linkedSeamId READ linkedSeamId WRITE setLinkedSeamId NOTIFY linkedSeamIdChanged)

    Q_PROPERTY(bool seamIncludesLinkedSeams READ seamIncludesLinkedSeams NOTIFY statisticsChanged)

    Q_PROPERTY(unsigned int seamIo READ seamIo NOTIFY statisticsChanged)

    Q_PROPERTY(unsigned int seamNio READ seamNio NOTIFY statisticsChanged)

    Q_PROPERTY(double seamIoInPercent READ seamIoInPercent NOTIFY statisticsChanged)

    Q_PROPERTY(double seamNioInPercent READ seamNioInPercent NOTIFY statisticsChanged)

    Q_PROPERTY(unsigned int seamIoIncludingLinkedSeams READ seamIoIncludingLinkedSeams NOTIFY statisticsChanged)

    Q_PROPERTY(unsigned int seamNioIncludingLinkedSeams READ seamNioIncludingLinkedSeams NOTIFY statisticsChanged)

    Q_PROPERTY(double seamIoInPercentIncludingLinkedSeams READ seamIoInPercentIncludingLinkedSeams NOTIFY statisticsChanged)

    Q_PROPERTY(double seamNioInPercentIncludingLinkedSeams READ seamNioInPercentIncludingLinkedSeams NOTIFY statisticsChanged)

public:
    explicit ResultsStatisticsSeamModel(QObject* parent = nullptr);
    ~ResultsStatisticsSeamModel() override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    precitec::storage::ResultsStatisticsController* resultsStatisticsController() const
    {
        return m_resultsStatisticsController;
    }
    void setResultsStatisticsController(ResultsStatisticsController* controller);

    const QUuid& seamSeriesId() const
    {
        return m_seamSeriesId;
    }
    void setSeamSeriesId(const QUuid& id);

    const QUuid& seamId() const
    {
        return m_seamId;
    }
    void setSeamId(const QUuid& id);

    const QUuid& linkedSeamId() const
    {
        return m_linkedSeamId;
    }
    void setLinkedSeamId(const QUuid& id);

    bool seamIncludesLinkedSeams() const;

    unsigned int seamIo() const;

    unsigned int seamNio() const;

    double seamIoInPercent() const;

    double seamNioInPercent() const;

    unsigned int seamIoIncludingLinkedSeams() const;

    unsigned int seamNioIncludingLinkedSeams() const;

    double seamIoInPercentIncludingLinkedSeams() const;

    double seamNioInPercentIncludingLinkedSeams() const;

Q_SIGNALS:
    void resultsStatisticsControllerChanged();
    void seamSeriesIdChanged();
    void seamIdChanged();
    void linkedSeamIdChanged();
    void statisticsChanged();

private:
    void updateModel();

    QUuid m_seamSeriesId;
    QUuid m_seamId;
    QUuid m_linkedSeamId;
    SeamStatistics m_seamStatistics;

    ResultsStatisticsController* m_resultsStatisticsController = nullptr;
    QMetaObject::Connection m_resultsStatisticsControllerDestroyed;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ResultsStatisticsSeamModel*)

