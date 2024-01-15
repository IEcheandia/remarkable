#pragma once

#include <QAbstractListModel>

class IntervalErrorConfigModelTest;

namespace precitec
{
namespace storage
{

class IntervalError;
class QualityNorm;
class QualityNormResult;
class Seam;

}
namespace gui
{
namespace components
{
namespace plotter
{

class DataSet;
class InfiniteSet;

}
}

class IntervalErrorConfigModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::IntervalError *intervalError READ intervalError WRITE setIntervalError NOTIFY intervalErrorChanged)

    Q_PROPERTY(precitec::storage::QualityNorm* qualityNorm READ qualityNorm WRITE setQualityNorm NOTIFY qualityNormChanged)

    Q_PROPERTY(bool qualityNormAvailable READ qualityNormAvailable NOTIFY qualityNormChanged)

    Q_PROPERTY(bool qualityNormResultAvailable READ qualityNormResultAvailable NOTIFY qualityNormResultChanged)

    Q_PROPERTY(precitec::storage::Seam* currentSeam READ currentSeam WRITE setCurrentSeam NOTIFY currentSeamChanged)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* visualReference READ visualReference NOTIFY visualReferenceChanged)

public:
    explicit IntervalErrorConfigModel(QObject *parent = nullptr);
    ~IntervalErrorConfigModel() override;

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

    precitec::storage::QualityNorm* qualityNorm() const
    {
        return m_qualityNorm;
    }
    void setQualityNorm(precitec::storage::QualityNorm* qualityNorm);

    bool qualityNormAvailable() const
    {
        return m_qualityNorm;
    }

    bool qualityNormResultAvailable() const
    {
        return m_qualityNormResult;
    }

    precitec::storage::Seam* currentSeam() const
    {
        return m_currentSeam;
    }
    void setCurrentSeam(precitec::storage::Seam* seam);

    precitec::gui::components::plotter::DataSet* visualReference() const
    {
        return m_visualReference;
    }

    Q_INVOKABLE void setVisualReference(precitec::gui::components::plotter::DataSet *ds);
    Q_INVOKABLE void setMinFromReference(uint level);
    Q_INVOKABLE void setMaxFromReference(uint level);

Q_SIGNALS:
    void intervalErrorChanged();
    void qualityNormChanged();
    void qualityNormResultChanged();
    void currentSeamChanged();
    void visualReferenceChanged();

private:
    void updateLowerBoundary();
    void updateUpperBoundary();
    void updateData(QVector<int> &roles);
    void updateQualityNormResult();
    void updateQualityNormValues();
    qreal qualityNormMin(int level) const;
    qreal qualityNormMax(int level) const;
    qreal qualityNormThreshold(int level) const;
    qreal qualityNormSecondThreshold(int level) const;

    precitec::gui::components::plotter::DataSet *m_visualReference = nullptr;
    std::vector<precitec::gui::components::plotter::InfiniteSet*> m_lowerBoundaries;
    std::vector<precitec::gui::components::plotter::InfiniteSet*> m_upperBoundaries;
    std::vector<precitec::gui::components::plotter::InfiniteSet*> m_shiftedLowerBoundaries;
    std::vector<precitec::gui::components::plotter::InfiniteSet*> m_shiftedUpperBoundaries;
    precitec::storage::IntervalError* m_intervalError = nullptr;
    QMetaObject::Connection m_destroyedConnection;
    precitec::storage::QualityNorm* m_qualityNorm = nullptr;
    QMetaObject::Connection m_destroyQualityNormConnection;
    precitec::storage::QualityNormResult* m_qualityNormResult = nullptr;
    QMetaObject::Connection m_qualityNormDestroyedConnection;
    precitec::storage::Seam* m_currentSeam = nullptr;
    QMetaObject::Connection m_seamDestroyedConnection;
    QMetaObject::Connection m_resultValueChangedConnection;
    QMetaObject::Connection m_minChangedConnection;
    QMetaObject::Connection m_maxChangedConnection;
    QMetaObject::Connection m_thresholdChangedConnection;
    QMetaObject::Connection m_secondThresholdChangedConnection;

    friend IntervalErrorConfigModelTest;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::IntervalErrorConfigModel*)


