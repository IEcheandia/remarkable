#include "productInstanceTransferHandler.h"
#include "emptyProductInstanceCreator.h"
#include "seamProduceInstanceCopyHandler.h"

#include "product.h"
#include "seam.h"
#include "seamSeries.h"

#include <set>

#include <QDir>
#include <QFile>
#include <QString>

using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SeamSeries;

using precitec::gui::EmptyProductInstanceCreator;
using precitec::gui::SeamProduceInstanceCopyHandler;

namespace precitec::gui
{

ProductInstanceTransferHandler::ProductInstanceTransferHandler(QObject *parent) : QObject(parent)
{
}

void ProductInstanceTransferHandler::setDirectory(const QString &directory)
{
    m_directory = directory;
}

void ProductInstanceTransferHandler::setSourceProductInstanceId(const QUuid &id)
{
    m_sourceProductInstanceId = id;
}

void ProductInstanceTransferHandler::setSourceSerialNumber(quint32 serialNumber)
{
    m_sourceSerialNumber = serialNumber;
}

void ProductInstanceTransferHandler::setSourceProduct(precitec::storage::Product *product)
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
            connect(m_sourceProduct, &QObject::destroyed, this,[this]
                    {
                        m_sourceProduct = nullptr;
                    });
    }
}

void ProductInstanceTransferHandler::setTargetProduct(precitec::storage::Product *product)
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
            connect(m_targetProduct, &QObject::destroyed, this, [this]
                    {
                        m_targetProduct = nullptr;
                    });
    }
}

bool ProductInstanceTransferHandler::transfer()
{
    if (!arePropertiesInitialized())
    {
        return false;
    }

    if (!createEmptyTargetProductInstance())
    {
        return false;
    }

    m_targetProductInstanceDirectory = productInstanceDirectory(m_targetProduct,m_targetProductInstanceId,m_sourceSerialNumber);

    conductTransfer();

    return true;
}

bool ProductInstanceTransferHandler::arePropertiesInitialized()
{
    if (!m_directory.isEmpty() &&
        !m_sourceProductInstanceId.isNull() &&
        m_sourceProduct != nullptr &&
        m_targetProduct != nullptr &&
        doDirectoriesExist())
    {
        return true;
    }

    return false;
}

bool ProductInstanceTransferHandler::doDirectoriesExist()
{
    if (!QDir(productInstanceDirectory(m_sourceProduct, m_sourceProductInstanceId, m_sourceSerialNumber)).exists())
    {
        return false;
    }

    if (!QDir(productDirectory(m_targetProduct)).exists())
    {
        return false;
    }

    return true;
}

QString ProductInstanceTransferHandler::productInstanceDirectory(precitec::storage::Product *product,
                                                                    const QUuid &productId,
                                                                    const quint32 productSerialNumber) const
{
    return productDirectory(product) + "/" + productInstanceDirectoryName(productId, productSerialNumber);
}

QString ProductInstanceTransferHandler::productDirectory(precitec::storage::Product *product) const
{
    return m_directory + "/" + productDirectoryName(product);
}

QString ProductInstanceTransferHandler::productDirectoryName(precitec::storage::Product *product)
{
    if (product == nullptr)
    {
        return {};
    }

    return (product->isDefaultProduct() ? product->name() : product->uuid().toString(QUuid::WithoutBraces));
}

QString ProductInstanceTransferHandler::productInstanceDirectoryName(const QUuid &productId,
                                                                        const quint32 productSerialNumber)
{
    return productId.toString(QUuid::WithoutBraces) + "-SN-" + QString::number(productSerialNumber);
}

bool ProductInstanceTransferHandler::createEmptyTargetProductInstance()
{
    EmptyProductInstanceCreator emptyProductInstanceCreator;

    emptyProductInstanceCreator.setProduct(m_targetProduct);
    emptyProductInstanceCreator.setProductDirectory(productDirectory(m_targetProduct));
    emptyProductInstanceCreator.setSerialNumber(m_sourceSerialNumber);
    m_targetProductInstanceId = QUuid::createUuid();
    emptyProductInstanceCreator.setProductInstanceId(m_targetProductInstanceId);

    return emptyProductInstanceCreator.create();
}

void ProductInstanceTransferHandler::conductTransfer()
{
    SeamProduceInstanceCopyHandler::SeamProduceInstanceInfo source;
    source.productUuid = m_sourceProduct->uuid();
    source.productName = m_sourceProduct->name();
    source.productInstanceDirectory =
        productInstanceDirectory(m_sourceProduct, m_sourceProductInstanceId, m_sourceSerialNumber);
    source.productInstanceUuid = m_sourceProductInstanceId;
    source.serialNumber = m_sourceSerialNumber;
    source.productType = m_sourceProduct->type();
    source.seamSeriesNumber = 0;
    source.seamNumber = 0;

    SeamProduceInstanceCopyHandler::SeamProduceInstanceInfo target;
    target.productUuid = m_targetProduct->uuid();
    target.productName = m_targetProduct->name();
    target.productInstanceDirectory =
        productInstanceDirectory(m_targetProduct, m_targetProductInstanceId, m_sourceSerialNumber);
    target.productInstanceUuid = m_targetProductInstanceId;
    target.serialNumber = m_sourceSerialNumber;
    target.productType = m_targetProduct->type();
    target.seamSeriesNumber = 0;
    target.seamNumber = 0;

    SeamProduceInstanceCopyHandler seamProduceInstanceCopyHandler;

    std::set<std::pair<quint32, quint32>> allTargetSeamSeriesSeamPairs;
    for (const auto &seam: m_targetProduct->seams())
    {
        allTargetSeamSeriesSeamPairs.emplace(std::make_pair(seam->seamSeries()->number(), seam->number()));
    }
    for (const auto &seam: m_sourceProduct->seams())
    {
        if (allTargetSeamSeriesSeamPairs.find(std::make_pair(seam->seamSeries()->number(), seam->number())) !=
            allTargetSeamSeriesSeamPairs.end())
        {
            source.seamSeriesNumber = target.seamSeriesNumber = seam->seamSeries()->number();
            source.seamNumber = target.seamNumber = seam->number();
            seamProduceInstanceCopyHandler.setSource(source);
            seamProduceInstanceCopyHandler.setTarget(target);
            seamProduceInstanceCopyHandler.copy();
        }
    }
}

ProductInstanceTransferHandler::~ProductInstanceTransferHandler()
{
}

QString ProductInstanceTransferHandler::targetFullProductInstanceDirectory() const
{
    return m_targetProductInstanceDirectory;
}

}