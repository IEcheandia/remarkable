#pragma once

#include "overlyingErrorModel.h"

class SeamSeriesErrorModelTest;

namespace precitec
{
namespace storage
{

class SeamSeries;
class AttributeModel;
class SeamSeriesError;

}
namespace gui
{

class SeamSeriesErrorModel : public OverlyingErrorModel
{
    Q_OBJECT

    /**
     * The currently used seam which holds the error
     **/
    Q_PROPERTY(precitec::storage::SeamSeries *currentSeamSeries READ currentSeamSeries WRITE setCurrentSeamSeries NOTIFY currentSeamSeriesChanged)


public:
    explicit SeamSeriesErrorModel(QObject *parent = nullptr);
    ~SeamSeriesErrorModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    precitec::storage::SeamSeries *currentSeamSeries() const
    {
        return m_currentSeamSeries;
    }
    void setCurrentSeamSeries(precitec::storage::SeamSeries *seamSeries);

    Q_INVOKABLE precitec::storage::SeamSeriesError *createError(precitec::gui::OverlyingErrorModel::ErrorType errorType);

    Q_INVOKABLE void removeError(precitec::storage::SeamSeriesError *error);

Q_SIGNALS:
    void currentSeamSeriesChanged();

private:
    precitec::storage::SeamSeries *m_currentSeamSeries = nullptr;
    QMetaObject::Connection m_destroyConnection;

    friend SeamSeriesErrorModelTest;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::SeamSeriesErrorModel*)
