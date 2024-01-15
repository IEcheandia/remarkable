#pragma once

#include <QAbstractItemModel>

class QUuid;
class OverlyingErrorModelTest;

namespace precitec
{
namespace storage
{

class AttributeModel;

}
namespace gui
{

class OverlyingErrorModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * The AttribueModel providing the Attribute description required when creating a new Error Parameter.
     **/
    Q_PROPERTY(precitec::storage::AttributeModel *attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged)

public:
    explicit OverlyingErrorModel(QObject *parent = nullptr);
    ~OverlyingErrorModel() override;

    enum class ErrorType {
        ConsecutiveTypeErrors,
        AccumulatedTypeErrors,
        ConsecutiveSeamErrors,
        AccumulatedSeamErrors
    };
    Q_ENUM(ErrorType)

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    precitec::storage::AttributeModel *attributeModel() const
    {
        return m_attributeModel;
    }
    void setAttributeModel(precitec::storage::AttributeModel *model);

Q_SIGNALS:
    void attributeModelChanged();
    void markAsChanged();

protected:
    QUuid variantId(ErrorType type) const;
    QString name(ErrorType type) const;
    QString nameFromId(const QUuid &id) const;
    bool isTypeError(const QUuid &id) const;

private:
    precitec::storage::AttributeModel *m_attributeModel = nullptr;
    QMetaObject::Connection m_attributeModelDestroyedConnection;

    friend OverlyingErrorModelTest;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::OverlyingErrorModel*)




