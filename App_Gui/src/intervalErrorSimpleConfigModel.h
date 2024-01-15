
#pragma once

#include <QAbstractListModel>

namespace precitec
{
namespace storage
{

class Seam;
class IntervalError;
class QualityNormResult;

}
namespace gui
{

class IntervalErrorSimpleConfigModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::IntervalError *intervalError READ intervalError WRITE setIntervalError NOTIFY intervalErrorChanged)

    Q_PROPERTY(precitec::storage::QualityNormResult* qualityNormResult READ qualityNormResult WRITE setQualityNormResult NOTIFY qualityNormResultChanged)

    Q_PROPERTY(bool qualityNormResultAvailable READ qualityNormResultAvailable NOTIFY qualityNormResultChanged)

    Q_PROPERTY(precitec::storage::Seam* currentSeam READ currentSeam WRITE setCurrentSeam NOTIFY currentSeamChanged)

public:
    explicit IntervalErrorSimpleConfigModel(QObject *parent = nullptr);
    ~IntervalErrorSimpleConfigModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex & index, const QVariant &value, int role) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    precitec::storage::IntervalError *intervalError() const
    {
        return m_intervalError;
    }
    void setIntervalError(precitec::storage::IntervalError *intervalError);

    precitec::storage::QualityNormResult* qualityNormResult() const
    {
        return m_qualityNormResult;
    }
    void setQualityNormResult(precitec::storage::QualityNormResult* qualityNormResult);

    bool qualityNormResultAvailable() const
    {
        return m_qualityNormResult;
    }

    precitec::storage::Seam* currentSeam() const
    {
        return m_currentSeam;
    }
    void setCurrentSeam(precitec::storage::Seam* seam);

    Q_INVOKABLE void setValueFromQualityNorms(int level);

Q_SIGNALS:
    void intervalErrorChanged();
    void qualityNormResultChanged();
    void currentSeamChanged();

private:
    qreal qualityNormMin(int level) const;
    qreal qualityNormMax(int level) const;
    qreal qualityNormThreshold(int level) const;
    qreal qualityNormSecondThreshold(int level) const;
    void updateQualityNormValues();

    precitec::storage::IntervalError* m_intervalError = nullptr;
    QMetaObject::Connection m_intervalErrorDestroyedConnection;

    precitec::storage::QualityNormResult* m_qualityNormResult = nullptr;
    QMetaObject::Connection m_qualityNormDestroyedConnection;

    precitec::storage::Seam* m_currentSeam = nullptr;
    QMetaObject::Connection m_seamDestroyedConnection;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::IntervalErrorSimpleConfigModel*)


