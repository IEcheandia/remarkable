#pragma once

#include "event/storageUpdate.interface.h"
#include "event/dbNotification.interface.h"

namespace precitec
{
namespace storage
{
class DbServer;
class ProductModel;

class StorageUpdateServer : public interface::TStorageUpdate<interface::AbstractInterface>
{
public:
    explicit StorageUpdateServer();
    ~StorageUpdateServer() override;

    void filterParameterUpdated(Poco::UUID measureTaskId, std::vector<std::shared_ptr<interface::FilterParameter>> filterParameters) override;
    void reloadProduct(Poco::UUID productId) override;
    void filterParameterCreated(Poco::UUID measureTaskId, std::vector<std::shared_ptr<interface::FilterParameter>> filterParameter) override;

    void setProductModel(const std::shared_ptr<ProductModel> &products)
    {
        m_products = products;
    }

    const std::shared_ptr<ProductModel> &productModel() const
    {
        return m_products;
    }

    void setDbNotificationProxy(const std::shared_ptr<interface::TDbNotification<interface::AbstractInterface>> &dbNotification)
    {
        m_dbNotification = dbNotification;
    }

    const std::shared_ptr<interface::TDbNotification<interface::AbstractInterface>> &dbNotificationProxy() const
    {
        return m_dbNotification;
    }

    void setDbServer(const std::shared_ptr<DbServer>& dbServer)
    {
        m_dbServer = dbServer;
    }

private:
    std::shared_ptr<ProductModel> m_products;
    std::shared_ptr<interface::TDbNotification<interface::AbstractInterface>> m_dbNotification;
    std::shared_ptr<DbServer> m_dbServer;
};

}
}
