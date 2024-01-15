#pragma once

#include <QObject>
#include <QModelIndex>
#include <QVariantList>
#include <QMetaType>

class AbstractDataExchangeControllerTest;

namespace precitec
{
namespace storage
{
class ProductModel;
class Product;
class Seam;
}
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

/**
 * Controller to get the products and seams to the figure editor. This enables the figure editor to import information from products or seams.
 **/
class AbstractDataExchangeController : public QObject
{
    Q_OBJECT
    /**
     * Product model
     * Is used to be able to select a product and get the product to the figure editor.
     **/
    Q_PROPERTY(precitec::storage::ProductModel* productModel READ productModel WRITE setProductModel NOTIFY productModelChanged)

    /**
     * Product model index
     * Index to select and get the right product from the product model.
     **/
    Q_PROPERTY(QModelIndex productModelIndex READ productModelIndex WRITE setProductModelIndex NOTIFY productModelIndexChanged)

    /**
     * Product
     * Holds the selected product form the product model to get the all seams of the product.
     **/
    Q_PROPERTY(precitec::storage::Product* product READ product WRITE setProduct NOTIFY productChanged)

    /**
     * Seams
     * Holds all seams from the product to be able to select a seam.
     **/
    Q_PROPERTY(QVariantList seams READ seams WRITE setSeams NOTIFY seamsChanged)

    /**
     * Seam list index
     * Index to select and get the right seam from all seams.
     **/
    Q_PROPERTY(int seamListIndex READ seamListIndex WRITE setSeamListIndex NOTIFY seamListIndexChanged)

    /**
     * Seam
     * Holds the selected seam to get all necessary seam information.
     **/
    Q_PROPERTY(precitec::storage::Seam* seam READ seam WRITE setSeam NOTIFY seamChanged)

public:
    ~AbstractDataExchangeController() override;

    precitec::storage::ProductModel* productModel() const
    {
        return m_productModel;
    }
    void setProductModel(precitec::storage::ProductModel* productModel);

    QModelIndex productModelIndex() const
    {
        return m_productModelIndex;
    }
    void setProductModelIndex(const QModelIndex& modelIndex);

    precitec::storage::Product* product() const
    {
        return m_product;
    }
    void setProduct(precitec::storage::Product* newProduct);

    QVariantList seams() const
    {
        return m_seams;
    }
    void setSeams(const QVariantList& seams);

    int seamListIndex() const
    {
        return m_seamListIndex;
    }
    void setSeamListIndex(int newIndex);

    precitec::storage::Seam* seam() const
    {
        return m_seam;
    }
    void setSeam(precitec::storage::Seam* newSeam);

Q_SIGNALS:
    void productModelChanged();
    void productModelIndexChanged();
    void productChanged();
    void seamsChanged();
    void seamListIndexChanged();
    void seamChanged();

protected:
    explicit AbstractDataExchangeController(QObject* parent = nullptr);

private:
    void setProductFromProductModel();
    void setSeamListFromProduct();
    void setSeamFromList();

    precitec::storage::ProductModel* m_productModel = nullptr;
    QMetaObject::Connection m_productModelDestroyedConnection;
    QModelIndex m_productModelIndex;

    precitec::storage::Product* m_product = nullptr;
    QMetaObject::Connection m_productDestroyedConnection;

    QVariantList m_seams;
    int m_seamListIndex{-1};

    precitec::storage::Seam* m_seam = nullptr;
    QMetaObject::Connection m_seamDestroyedConnection;

    friend AbstractDataExchangeControllerTest;
};

}
}
}
}
