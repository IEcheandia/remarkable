#pragma once

#include <precitec/change.h>

namespace precitec
{

namespace storage
{
class Product;
}

namespace gui
{

class ProductDeletedChangeEntry : public components::userLog::Change
{
    Q_OBJECT
    Q_PROPERTY(precitec::storage::Product *product READ product CONSTANT)
public:
    Q_INVOKABLE ProductDeletedChangeEntry(QObject *parent = nullptr);
    ProductDeletedChangeEntry(storage::Product *product, QObject *parent = nullptr);
    ~ProductDeletedChangeEntry() override;

    QUrl detailVisualization() const override;

    void initFromJson(const QJsonObject &data) override;
    QJsonObject data() const override;

    storage::Product *product() const
    {
        return m_product;
    }

private:
    storage::Product *m_product = nullptr;
    QJsonObject m_data;
};

}
}
