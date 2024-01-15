#pragma once

#include "event/resultType.h"

#include <QAbstractListModel>

namespace precitec
{
namespace storage
{

class ResultsStatisticsController;
class ErrorSettingModel;

class ResultsStatisticsAbstractErrorModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::ResultsStatisticsController* resultsStatisticsController READ resultsStatisticsController WRITE setResultsStatisticsController NOTIFY resultsStatisticsControllerChanged)

    Q_PROPERTY(precitec::storage::ErrorSettingModel* errorSettingModel READ errorSettingModel WRITE setErrorSettingModel NOTIFY errorSettingModelChanged)

    Q_PROPERTY(bool empty READ empty NOTIFY emptyChanged)

public:
    ~ResultsStatisticsAbstractErrorModel() override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    precitec::storage::ResultsStatisticsController* resultsStatisticsController() const
    {
        return m_resultsStatisticsController;
    }
    void setResultsStatisticsController(ResultsStatisticsController* controller);

    precitec::storage::ErrorSettingModel* errorSettingModel() const
    {
        return m_errorSettingModel;
    }
    void setErrorSettingModel(ErrorSettingModel* errorSettingModel);

    bool empty() const
    {
        return m_nios.empty();
    }

Q_SIGNALS:
    void resultsStatisticsControllerChanged();
    void errorSettingModelChanged();
    void emptyChanged();

protected:
    explicit ResultsStatisticsAbstractErrorModel(QObject* parent = nullptr);
    virtual void updateModel() = 0;
    void clear();
    void setNios(const std::map<precitec::interface::ResultType, unsigned int>& nios);

private:
    std::map<precitec::interface::ResultType, unsigned int> m_nios;
    unsigned int m_nioCount = 0;

    ResultsStatisticsController* m_resultsStatisticsController = nullptr;
    QMetaObject::Connection m_resultsStatisticsControllerDestroyed;

    ErrorSettingModel* m_errorSettingModel = nullptr;
    QMetaObject::Connection m_errorSettingModelDestroyed;
};

}
}

