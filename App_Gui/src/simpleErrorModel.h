#pragma once

#include <QAbstractItemModel>

class SimpleErrorModelTest;

namespace precitec
{

namespace storage
{

class Seam;
class SeamError;
class IntervalError;
class AttributeModel;

}

namespace gui
{

class SimpleErrorModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The currently used seam which holds the error
     **/
    Q_PROPERTY(precitec::storage::Seam *currentSeam READ currentSeam WRITE setCurrentSeam NOTIFY currentSeamChanged)

    /**
     * The AttribueModel providing the Attribute description required when creating a new error Parameter.
     **/
    Q_PROPERTY(precitec::storage::AttributeModel *attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged)

public:
    explicit SimpleErrorModel(QObject *parent = nullptr);
    ~SimpleErrorModel() override;

    enum class ErrorType {
        LengthOutsideStaticBoundary,
        LengthOutsideReferenceBoundary,
        AccumulatedLengthOutsideStaticBoundary,
        AccumulatedLengthOutsideReferenceBoundary,
        LengthInsideStaticBoundary,
        LengthInsideReferenceBoundary,
        AreaStaticBoundary,
        AreaReferenceBoundary,
        AccumulatedAreaStaticBoundary,
        AccumulatedAreaReferenceBoundary,
        PeakStaticBoundary,
        PeakReferenceBoundary,
        DualOutlierStaticBoundary,
        DualOutlierReferenceBoundary
    };
    Q_ENUM(ErrorType)

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    precitec::storage::Seam *currentSeam() const
    {
        return m_currentSeam;
    }
    void setCurrentSeam(precitec::storage::Seam *seam);

    precitec::storage::AttributeModel *attributeModel() const
    {
        return m_attributeModel;
    }
    void setAttributeModel(precitec::storage::AttributeModel *model);

Q_SIGNALS:
    void currentSeamChanged();
    void attributeModelChanged();
    void markAsChanged();

protected:
    precitec::storage::SeamError *addError(ErrorType errorType);
    precitec::storage::IntervalError *addIntervalError(ErrorType errorType);
    QString nameFromId(const QUuid &id) const;

private:
    precitec::storage::Seam *m_currentSeam = nullptr;
    QMetaObject::Connection m_destroyConnection;
    precitec::storage::AttributeModel *m_attributeModel = nullptr;
    QMetaObject::Connection m_attributeModelDestroyedConnection;

    friend SimpleErrorModelTest;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::SimpleErrorModel*)

