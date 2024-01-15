#pragma once

#include <precitec/change.h>

#include <QFileInfo>
#include <QUuid>
#include <QJsonArray>

#include <forward_list>

namespace precitec
{

namespace storage
{
class Product;
}

namespace gui
{

class ProductInstancesDeletedChangeEntry :  public components::userLog::Change
{
    Q_OBJECT
public:
    Q_INVOKABLE ProductInstancesDeletedChangeEntry(QObject *parent = nullptr);
    ProductInstancesDeletedChangeEntry(storage::Product *product, const std::forward_list<std::tuple<QFileInfo, QString, QDateTime>> &instances, QObject *parent = nullptr);
    ~ProductInstancesDeletedChangeEntry() override;

protected:
    QJsonObject data() const override;
    void initFromJson(const QJsonObject &data) override;

private:
    QUuid m_productId;
    QString m_productName;
    QJsonArray m_instances;
};

}
}
