#pragma once

#include <QAbstractListModel>

namespace precitec
{
namespace storage
{

class ResultSetting;
class ProductModel;
class AttributeModel;

/**
 * This model provides all NioSettings.
 * NioSettings are used for configuration of the nio's, mainly the plotter values
 */
class NioSettingModel : public QAbstractListModel {

    Q_OBJECT
    /**
     * The ProductModel from which the Products are received
     **/
    Q_PROPERTY(precitec::storage::ProductModel *productModel READ productModel WRITE setProductModel NOTIFY productModelChanged)
    /**
     * The AttribueModel providing the Attribute description required when creating a new error Parameter.
     **/
    Q_PROPERTY(precitec::storage::AttributeModel *attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged)

public:
    explicit NioSettingModel(QObject *parent = nullptr);
    ~NioSettingModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    ProductModel *productModel() const
    {
        return m_productModel;
    }
    void setProductModel(ProductModel *productModel);

    AttributeModel *attributeModel() const
    {
        return m_attributeModel;
    }
    void setAttributeModel(AttributeModel *model);

    /**
     * returns the index of item in the list, given by the key (the enum)
     */
    Q_INVOKABLE int findIndex(int value);

    /**
     * returns the name of item in the list, given by the key (the enum)
     */
    QString findName(int value);

    /**
     * search the graph's data of all products and get the 'ErrorType'
     * this is a hack for collecting errors because we don't have errortypes yet (12/2018)
     **/
    void createGraphNiosList();

    /**
    * @returns A name for the given enumType, if the enum is unknown it returns the enumType converted to string.
    **/
    QString nameForResult(int enumType);

Q_SIGNALS:
    void productModelChanged();
    void attributeModelChanged();

private:
   void createAndAddItem(const QUuid &uuid, int enumType);
   ProductModel *m_productModel = nullptr;
   AttributeModel *m_attributeModel = nullptr;
   std::vector<ResultSetting*> m_nioItems;

};

}
}
Q_DECLARE_METATYPE(precitec::storage::NioSettingModel*)
