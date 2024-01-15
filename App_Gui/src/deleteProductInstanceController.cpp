#include "deleteProductInstanceController.h"

#include "product.h"
#include "productInstancesDeletedChangeEntry.h"
#include "productInstanceModel.h"

#include <forward_list>

#include <precitec/userLog.h>

namespace precitec
{

using storage::ProductInstanceModel;

namespace gui
{

DeleteProductInstanceController::DeleteProductInstanceController(QObject* parent)
    : QObject(parent)
{
}

DeleteProductInstanceController::~DeleteProductInstanceController() = default;

void DeleteProductInstanceController::setProductInstanceModel(ProductInstanceModel* model)
{
    if (m_productInstanceModel == model)
    {
        return;
    }
    disconnect(m_productInstanceModelDestroyConnection);
    m_productInstanceModelDestroyConnection = {};
    m_productInstanceModel = model;

    if (m_productInstanceModel)
    {
        m_productInstanceModelDestroyConnection = connect(m_productInstanceModel, &QObject::destroyed, this, std::bind(&DeleteProductInstanceController::setProductInstanceModel, this, nullptr));
    }
    emit productInstanceModelChanged();
}

void DeleteProductInstanceController::setVideoRecorderProxy(const VideoRecorderProxy &proxy)
{
    if (m_videoRecorderProxy == proxy)
    {
        return;
    }
    m_videoRecorderProxy = proxy;
    emit videoRecorderProxyChanged();
}

void DeleteProductInstanceController::deleteAllChecked()
{
    if (!m_productInstanceModel || !m_videoRecorderProxy)
    {
        return;
    }
    std::forward_list<std::tuple<QFileInfo, QString, QDateTime>> itemsToDelete;
    for (int i = 0; i < m_productInstanceModel->rowCount(); i++)
    {
        auto index = m_productInstanceModel->index(i, 0);
        if (!index.data(Qt::UserRole + 7).toBool())
        {
            continue;
        }
        itemsToDelete.emplace_front(index.data(Qt::UserRole).value<QFileInfo>(), index.data(Qt::DisplayRole).toString(), index.data(Qt::UserRole + 1).toDateTime());
    }
    if (itemsToDelete.empty())
    {
        return;
    }
    components::userLog::UserLog::instance()->addChange(new ProductInstancesDeletedChangeEntry{m_productInstanceModel->product(), itemsToDelete});
    const bool liveMode = m_productInstanceModel->product() ? m_productInstanceModel->product()->isDefaultProduct() : false;
    std::vector<std::string> instancesToDelete;
    instancesToDelete.reserve(m_productInstanceModel->rowCount());
    std::transform(itemsToDelete.cbegin(), itemsToDelete.cend(), std::back_inserter(instancesToDelete), [] (const auto& tuple) { return std::get<QFileInfo>(tuple).absoluteFilePath().toStdString(); });
    if (liveMode)
    {
        m_videoRecorderProxy->deleteLiveModeProductInstances(instancesToDelete);
    } else
    {
        m_videoRecorderProxy->deleteAutomaticProductInstances(instancesToDelete);
    }
}

}
}
