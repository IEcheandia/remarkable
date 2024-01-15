#pragma once

#include "abstractMultiSeamDataModel.h"

namespace precitec
{
namespace storage
{

class ResultsSeriesLoader;

}
namespace gui
{

class SeamSeriesResultsModel : public AbstractMultiSeamDataModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::ResultsSeriesLoader* resultsSeriesLoader READ resultsSeriesLoader WRITE setResultsSeriesLoader NOTIFY resultsSeriesLoaderChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)

public:
    explicit SeamSeriesResultsModel(QObject* parent = nullptr);
    ~SeamSeriesResultsModel() override;

    precitec::storage::ResultsSeriesLoader* resultsSeriesLoader() const
    {
        return m_resultsSeriesLoader;
    }
    void setResultsSeriesLoader(precitec::storage::ResultsSeriesLoader* loader);

    bool isLoading() const
    {
        return m_loading;
    }

Q_SIGNALS:
    void resultsSeriesLoaderChanged();
    void loadingChanged();

private:
    void update();
    void setLoading(bool loading);
    void disableTooManyPoints(int resultIndex, int seamIndex);

    precitec::storage::ResultsSeriesLoader* m_resultsSeriesLoader = nullptr;
    QMetaObject::Connection m_resultsLoaderDestroyedConnection;

    bool m_loading = false;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::SeamSeriesResultsModel*)

