#include "resultsServer.h"

#include "productModel.h"
#include "product.h"
#include "seam.h"
#include "seamSeries.h"

using namespace precitec::interface;

using precitec::storage::ProductModel;

namespace precitec
{
namespace storage
{
ResultsServer::ResultsServer(QObject *parent)
    : QObject(parent)
{
}

ResultsServer::~ResultsServer() = default;

void ResultsServer::result(ResultIntArray const& value)
{
    emit resultsReceived(value);
}

void ResultsServer::result(ResultDoubleArray const& value)
{
    emit resultsReceived(value);
}

void ResultsServer::result(ResultRangeArray const& value)
{
    emit resultsReceived(value);
}

void ResultsServer::result(ResultRange1dArray const& value)
{
    emit resultsReceived(value);
}

void ResultsServer::result(ResultPointArray const& value)
{
    emit resultsReceived(value);
}

void ResultsServer::nio(NIOResult const& result)
{
    emit nioReceived(result);
}

void ResultsServer::result(const std::vector<precitec::interface::ResultDoubleArray> &results)
{
    emit combinedResultsReceived(results);
}

void ResultsServer::inspectionStarted(Poco::UUID productID, Poco::UUID instanceProductID, uint32_t productNr, MeasureTaskIDs measureTaskIDs, IntRange range, int p_oSeamNo, int triggerDeltaInMicrons, Poco::UUID seamId, const std::string &seamLabel)
{
    Q_UNUSED(measureTaskIDs)
    Q_UNUSED(range)
    Q_UNUSED(p_oSeamNo)
    Q_UNUSED(triggerDeltaInMicrons)

    if (!m_products)
    {
        return;
    }
    QMutexLocker lock{m_products->storageMutex()};

    auto p = findProduct(productID);
    if (!p)
    {
        return;
    }
    auto s = p->findSeam(QUuid::fromString(QString::fromStdString(seamId.toString())));
    if (!s)
    {
        return;
    }
    if (!seamLabel.empty())
    {
        const auto label = QString::fromStdString(seamLabel);
        auto link = s->findLink(label);
        if (!link)
        {
            p->setChangeTrackingEnabled(true);
            link = s->seamSeries()->createSeamLink(s, label);
            if (!link)
            {
                return;
            }
            link->moveToThread(s->thread());
            link->setParent(s->seamSeries());
            p->ensureAllFilterParameterSetsLoaded();
        }
        s = link;
    }
    lock.unlock();

    emit seamInspectionStarted(QPointer<precitec::storage::Seam>{s}, QUuid::fromString(QString::fromStdString(instanceProductID.toString())), productNr);
}

void ResultsServer::inspectionEnded(MeasureTaskIDs measureTaskIDs)
{
    Q_UNUSED(measureTaskIDs)
    // Note: the inspectionEnded is always invoked with an empty measureTaskIDs vector
    emit seamInspectionEnded();
}

void ResultsServer::inspectionAutomaticStart(Poco::UUID productID, Poco::UUID instanceProductID, const std::string &extendedProductInfo)
{
    if (!m_products)
    {
        return;
    }
    QMutexLocker lock{m_products->storageMutex()};
    auto p = findProduct(productID);
    if (!p)
    {
        return;
    }
    lock.unlock();

    emit productInspectionStarted(QPointer<precitec::storage::Product>{p}, QUuid::fromString(QString::fromStdString(instanceProductID.toString())), QString::fromStdString(extendedProductInfo));
}

void ResultsServer::inspectionAutomaticStop(Poco::UUID productID, Poco::UUID instanceProductID)
{
    Q_UNUSED(instanceProductID)
    QPointer<precitec::storage::Product> product;
    if (m_products)
    {
        QMutexLocker lock{m_products->storageMutex()};
        if (auto p = findProduct(productID))
        {
            product = p;
        }
    }
    emit productInspectionStopped(product);
}

Product *ResultsServer::findProduct(const Poco::UUID &id) const
{
    if (!m_products)
    {
        return nullptr;
    }

    return m_products->findProduct(QUuid::fromString(QString::fromStdString(id.toString())));
}

}
}
