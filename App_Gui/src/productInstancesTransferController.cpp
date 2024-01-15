#include "productInstancesTransferController.h"

#include "productInstanceModel.h"
#include "checkedFilterModel.h"

#include "product.h"

#include "productInstanceTransferHandler.h"
#include "productInstancesCacheEditor.h"

#include <QtConcurrent>

#include <limits.h>

using precitec::storage::Product;
using precitec::storage::ProductInstanceModel;

using precitec::gui::CheckedFilterModel;
using precitec::gui::ProductInstanceTransferHandler;

namespace precitec::gui
{

ProductInstancesTransferController::ProductInstancesTransferController(QObject *parent) : QObject(parent)
{
}

void ProductInstancesTransferController::setSourceProductInstanceModel(precitec::storage::ProductInstanceModel *model)
{
    if (m_sourceProductInstanceModel == model)
    {
        return;
    }
    disconnect(m_sourceProductInstanceModelDestroyedConnection);
    m_sourceProductInstanceModel = model;
    if (m_sourceProductInstanceModel)
    {
        m_sourceProductInstanceModelDestroyedConnection =
            connect(m_sourceProductInstanceModel, &QObject::destroyed, this,
                    [this] { m_sourceProductInstanceModel = nullptr; });
    }
}

void ProductInstancesTransferController::setIndexFilterModel(precitec::gui::CheckedFilterModel *model)
{
    if (m_indexFilterModel == model)
    {
        return;
    }
    disconnect(m_indexFilterModelDestroyedConnection);
    m_indexFilterModel = model;
    if (m_indexFilterModel)
    {
        m_indexFilterModelDestroyedConnection =
            connect(m_indexFilterModel, &QObject::destroyed, this, [this] { m_indexFilterModel = nullptr; });
    }
    emit indexFilterModelChanged();
}

void ProductInstancesTransferController::setTargetProduct(precitec::storage::Product *product)
{
    if (m_targetProduct == product)
    {
        return;
    }
    disconnect(m_targetProductDestroyedConnection);
    m_targetProduct = product;
    if (m_targetProduct)
    {
        m_targetProductDestroyedConnection =
            connect(m_targetProduct, &QObject::destroyed, this, [this] { m_targetProduct = nullptr; });
    }
    emit targetProductChanged();
}

void ProductInstancesTransferController::setSourceProduct(precitec::storage::Product *product)
{
    if (m_sourceProduct == product)
    {
        return;
    }
    disconnect(m_sourceProductDestroyedConnection);
    m_sourceProduct = product;
    if (m_sourceProduct)
    {
        m_sourceProductDestroyedConnection =
            connect(m_sourceProduct, &QObject::destroyed, this, [this] { m_sourceProduct = nullptr; });
    }
}

bool ProductInstancesTransferController::transfer()
{
    if (!arePublicPropertiesInitialised())
    {
        return false;
    }

    if (m_transferInProgress)
    {
        return false;
    }

    if (!checkAndInitializePrivateProperties())
    {
        return false;
    }

    transferAllConcurrently();
    return true;
}

bool ProductInstancesTransferController::arePublicPropertiesInitialised()
{
    return (m_targetProduct != nullptr && m_indexFilterModel != nullptr);
}

bool ProductInstancesTransferController::checkAndInitializePrivateProperties()
{
    if (auto sourceProductInstanceModel = m_indexFilterModel->sourceModel(); sourceProductInstanceModel == nullptr)
    {
        return false;
    }
    else
    {
        setSourceProductInstanceModel(qobject_cast<ProductInstanceModel *>(sourceProductInstanceModel));
    }

    if (auto sourceProduct = m_sourceProductInstanceModel->product(); sourceProduct == nullptr)
    {
        return false;
    }
    else
    {
        setSourceProduct(sourceProduct);
    }

    return true;
}

void ProductInstancesTransferController::transferAllConcurrently()
{
    setTransferInProgress(true);
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, [this]() { this->setTransferInProgress(false); });

    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);

    watcher->setFuture(QtConcurrent::run(this, &ProductInstancesTransferController::transferAll));
}

void ProductInstancesTransferController::transferAll()
{
    for (auto index = 0; index < m_indexFilterModel->rowCount(); index++)
    {
        transfer(index);
    }

    if (m_usingCacheMonitoring)
    {
        runProductInstanceCacheEditor();
    }
}

void ProductInstancesTransferController::transfer(quint32 indexFilterModelIndex)
{
    ProductInstanceTransferHandler productInstanceTransferHandler;

    productInstanceTransferHandler.setSourceProduct(m_sourceProduct);
    productInstanceTransferHandler.setTargetProduct(m_targetProduct);

    auto sourceProductInstanceIndex =
        m_indexFilterModel->mapToSource(m_indexFilterModel->index(indexFilterModelIndex, 0));
    quint32 sourceProductInstanceSerialNumber =
        m_sourceProductInstanceModel->data(sourceProductInstanceIndex, Qt::DisplayRole).toUInt();
    productInstanceTransferHandler.setSourceSerialNumber(sourceProductInstanceSerialNumber);
    QUuid sourceProductInstanceId =
        m_sourceProductInstanceModel->data(sourceProductInstanceIndex, Qt::UserRole + 9).toUuid();
    productInstanceTransferHandler.setSourceProductInstanceId(sourceProductInstanceId);

    productInstanceTransferHandler.setDirectory(m_sourceProductInstanceModel->directory());

    productInstanceTransferHandler.transfer();
    if ( auto targetProductInstanceDirectory = productInstanceTransferHandler.targetFullProductInstanceDirectory();
        !targetProductInstanceDirectory.isEmpty())
    {
        m_targetProductInstanceDirectories.emplace_back(targetProductInstanceDirectory);
    }
}

void ProductInstancesTransferController::setMonitoring(bool toUse)
{
    m_usingCacheMonitoring = toUse;
}

void ProductInstancesTransferController::runProductInstanceCacheEditor()
{
    precitec::gui::ProductInstancesCacheEditor cacheEditor(m_targetProduct->isDefaultProduct());
    cacheEditor.setProductInstanceMaxNumber(INT32_MAX);
    cacheEditor.add(m_targetProductInstanceDirectories);
}

void ProductInstancesTransferController::setTransferInProgress(bool transferInProgress)
{
    if (m_transferInProgress == transferInProgress)
    {
        return;
    }
    m_transferInProgress = transferInProgress;
    emit transferInProgressChanged();
}

ProductInstancesTransferController::~ProductInstancesTransferController()
{
}

}