#pragma once

#include "overlyingErrorModel.h"

class ProductErrorModelTest;

namespace precitec
{
namespace storage
{

class Product;
class AttributeModel;
class ProductError;

}
namespace gui
{

class ProductErrorModel : public OverlyingErrorModel
{
    Q_OBJECT

    /**
     * The currently used seam which holds the error
     **/
    Q_PROPERTY(precitec::storage::Product *currentProduct READ currentProduct WRITE setCurrentProduct NOTIFY currentProductChanged)


public:
    explicit ProductErrorModel(QObject *parent = nullptr);
    ~ProductErrorModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    precitec::storage::Product *currentProduct() const
    {
        return m_currentProduct;
    }
    void setCurrentProduct(precitec::storage::Product *product);

    Q_INVOKABLE precitec::storage::ProductError *createError(precitec::gui::OverlyingErrorModel::ErrorType errorType);

    Q_INVOKABLE void removeError(precitec::storage::ProductError *error);

Q_SIGNALS:
    void currentProductChanged();

private:
    precitec::storage::Product *m_currentProduct = nullptr;
    QMetaObject::Connection m_destroyConnection;

    friend ProductErrorModelTest;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::ProductErrorModel*)


