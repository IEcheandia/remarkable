#include "storageUpdateServer.h"
#include "abstractMeasureTask.h"
#include "parameter.h"
#include "parameterSet.h"
#include "product.h"
#include "productModel.h"
#include "compatibility.h"
#include "dbServer.h"

namespace precitec
{
namespace storage
{

StorageUpdateServer::StorageUpdateServer() = default;

StorageUpdateServer::~StorageUpdateServer() = default;

void StorageUpdateServer::filterParameterUpdated(Poco::UUID measureTaskId, std::vector<std::shared_ptr<interface::FilterParameter>> filterParameters)
{
    std::cout << "Storage App: StorageUpdateServer::filterParameterUpdated" << std::endl;
    if (!m_products)
    {
        return;
    }
    QMutexLocker lock{m_products->storageMutex()};
    const QUuid qtMeasureTaskId = compatibility::toQt(measureTaskId);
    ParameterSet *ps = nullptr;
    QUuid productId;
    for (auto p : m_products->products())
    {
        if (auto measureTask = p->findMeasureTask(qtMeasureTaskId))
        {
            ps = p->filterParameterSet(measureTask->graphParamSet());
            productId = p->uuid();
            break;
        }
    }
    if (!ps)
    {
        return;
    }
    std::set<Poco::UUID> updatedFilterParameters;
    const auto parameters = ps->parameters();
    for (auto filterParameter : filterParameters)
    {
        const auto id = compatibility::toQt(filterParameter->parameterID());
        auto it = std::find_if(parameters.begin(), parameters.end(), [id] (auto parameter) { return parameter->uuid() == id; });
        if (it == parameters.end())
        {
            continue;
        }
        auto parameter = *it;
        switch (filterParameter->type())
        {
            case TChar:
            case TByte:
                parameter->setValue(filterParameter->value<char>());
                break;
            case TInt:
                parameter->setValue(filterParameter->value<int>());
                break;
            case TUInt:
                parameter->setValue(filterParameter->value<uint>());
                break;
            case TBool:
                parameter->setValue(filterParameter->value<bool>());
                break;
            case TFloat:
                parameter->setValue(filterParameter->value<float>());
                break;
            case TDouble:
                parameter->setValue(filterParameter->value<double>());
                break;
            case TString:
                parameter->setValue(QString::fromStdString(filterParameter->value<std::string>()));
                break;
            case TNumTypes:
            case TOpMode:
            case TUnknown:
                break;
        }
        updatedFilterParameters.insert(compatibility::toPoco(parameter->filterId()));
    }
    lock.unlock();
    if (m_dbNotification)
    {
        for (const auto &filter : updatedFilterParameters)
        {
            m_dbNotification->setupFilterParameter(measureTaskId, filter);
        }
        m_dbNotification->setupProduct(compatibility::toPoco(productId));
    }
}

void StorageUpdateServer::filterParameterCreated(Poco::UUID measureTaskId, std::vector<std::shared_ptr<interface::FilterParameter>> filterParameter)
{
    if (!m_products)
    {
        return;
    }
    QMutexLocker lock{m_products->storageMutex()};
    const QUuid qtMeasureTaskId = compatibility::toQt(measureTaskId);
    ParameterSet *ps = nullptr;
    QUuid productId;
    for (auto p : m_products->products())
    {
        if (auto measureTask = p->findMeasureTask(qtMeasureTaskId))
        {
            ps = p->filterParameterSet(measureTask->graphParamSet());
            productId = p->uuid();
            break;
        }
    }
    if (!ps)
    {
        return;
    }
    for (const auto &fp : filterParameter)
    {
        ps->createParameter(*fp.get());
    }
}

void StorageUpdateServer::reloadProduct(Poco::UUID productId)
{
    if (!m_products)
    {
        return;
    }
    // not executed from the ProductModel thread, we cannot reload from here, but need to trigger a message
    QMetaObject::invokeMethod(m_products.get(), std::bind(&ProductModel::reloadProduct, m_products.get(), compatibility::toQt(productId)), Qt::QueuedConnection);
}

}
}
