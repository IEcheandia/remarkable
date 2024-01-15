#pragma once

#include "overlyingError.h"

class TestProductError;

namespace precitec
{
namespace interface
{

class FilterParameter;

}
namespace storage
{

class Product;
enum class CopyMode;

class ProductError : public OverlyingError
{
    Q_OBJECT

public:
    ProductError(QObject* parent = nullptr);
    ProductError(QUuid uuid, QObject* parent = nullptr);
    ~ProductError() override;

    /**
     * Duplicates this ProductError
     * @param mode selects whether the duplicate gets a new, random Uuid or the same Uuid as this object.
     * @param parent is used as the product of the duplicated ProductError.
     **/
    ProductError *duplicate(CopyMode mode, Product *parent) const;

    Product *product() const
    {
        return m_product;
    }
    void setProduct(Product *product);

    std::vector<std::shared_ptr<precitec::interface::FilterParameter> > toParameterList() const;

    static ProductError *fromJson(const QJsonObject &object, Product *parent);

    bool isChangeTracking() override;

Q_SIGNALS:
    void productChanged();

private:
    int getIntValue(const QString &name) const;
    std::string getStringValue(const QString& n) const;

    Product *m_product = nullptr;

    friend TestProductError;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::ProductError*)

