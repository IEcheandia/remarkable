#pragma once

#include "abstractSingleSeamDataModel.h"

namespace precitec
{

namespace storage
{

class ResultsLoader;

}
namespace gui
{

class ResultsDataSetModel : public AbstractSingleSeamDataModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::ResultsLoader* resultsLoader READ resultsLoader WRITE setResultsLoader NOTIFY resultsLoaderChanged)

public:
    explicit ResultsDataSetModel(QObject* parent = nullptr);
    ~ResultsDataSetModel() override;

    precitec::storage::ResultsLoader* resultsLoader() const
    {
        return m_resultsLoader;
    }

    void setResultsLoader(precitec::storage::ResultsLoader* loader);

    Q_INVOKABLE QVariant valueAtPosition(const QModelIndex& index, float position);

Q_SIGNALS:
    void resultsLoaderChanged();

private:
    void update();

    precitec::storage::ResultsLoader* m_resultsLoader = nullptr;
    QMetaObject::Connection m_resultsLoaderDestroyedConnection;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ResultsDataSetModel*)
