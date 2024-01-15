#pragma once

#include "simpleErrorModel.h"

namespace precitec
{

namespace storage
{

class IntervalError;
class QualityNorm;

}

namespace gui
{

class IntervalErrorModel : public SimpleErrorModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::QualityNorm* qualityNorm READ qualityNorm WRITE setQualityNorm NOTIFY qualityNormChanged)

    Q_PROPERTY(bool qualityNormAvailable READ qualityNormAvailable NOTIFY qualityNormChanged)

public:
    explicit IntervalErrorModel(QObject *parent = nullptr);
    ~IntervalErrorModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    precitec::storage::QualityNorm* qualityNorm() const
    {
        return m_qualityNorm;
    }
    void setQualityNorm(precitec::storage::QualityNorm* qualityNorm);

    bool qualityNormAvailable() const
    {
        return m_qualityNorm;
    }

    /**
     * Creates a new IntervalError
     * @returns new created IntervalError
     **/
    Q_INVOKABLE precitec::storage::IntervalError *createError(precitec::gui::SimpleErrorModel::ErrorType errorType);

    /**
     * Delete selected IntervalError
     **/
    Q_INVOKABLE void removeError(precitec::storage::IntervalError *error);

Q_SIGNALS:
    void qualityNormChanged();

private:
    void updateQualityNormResult();

    precitec::storage::QualityNorm* m_qualityNorm = nullptr;
    QMetaObject::Connection m_destroyQualityNormConnection;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::IntervalErrorModel*)

