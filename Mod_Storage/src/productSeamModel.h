#pragma once

#include <QAbstractListModel>

namespace precitec
{
namespace storage
{

class Product;
class Seam;

/**
 *
 * Model exposing all Seams of a Product
 **/
class ProductSeamModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The Product the model contains
     **/
    Q_PROPERTY(precitec::storage::Product *product READ product WRITE setProduct NOTIFY productChanged)
public:
    ProductSeamModel(QObject *parent = nullptr);
    ~ProductSeamModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent) const override;
    QHash<int, QByteArray> roleNames() const override;

    Product *product() const
    {
        return m_product;
    }
    void setProduct(Product *product);

Q_SIGNALS:
    void productChanged();

private:
    void initSeams();
    Product *m_product = nullptr;
    QMetaObject::Connection m_productDestroyedConnection;
    QMetaObject::Connection m_seamChangedConnection;
    std::vector<Seam*> m_seams;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ProductSeamModel*)
