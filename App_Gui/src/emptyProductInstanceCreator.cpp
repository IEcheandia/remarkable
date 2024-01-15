#include "emptyProductInstanceCreator.h"

#include "product.h"

#include <QDir>
#include <QFile>

using precitec::storage::Product;

namespace precitec::gui
{

EmptyProductInstanceCreator::EmptyProductInstanceCreator(QObject *parent) : QObject(parent)
{
}

void EmptyProductInstanceCreator::setProductDirectory(const QString &directory)
{
    if (m_productDirectory == directory)
    {
        return;
    }

    m_productDirectory = directory;
}

void EmptyProductInstanceCreator::setProduct(Product *product)
{
    if (m_product == product)
    {
        return;
    }
    disconnect(m_productDestroyedConnection);
    m_product = product;
    if (m_product)
    {
        m_productDestroyedConnection = connect(m_product, &QObject::destroyed, this, [this] { m_product = nullptr; });
    }
}

void EmptyProductInstanceCreator::setSerialNumber(quint32 serialNumber)
{
    if (m_serialNumber == serialNumber)
    {
        return;
    }

    m_serialNumber = serialNumber;
}

void EmptyProductInstanceCreator::setProductInstanceId(const QUuid &id)
{
    if (m_productInstanceId == id)
    {
        return;
    }

    m_productInstanceId = id;
}

bool EmptyProductInstanceCreator::create()
{
    if (!arePropertiesValid())
    {
        return false;
    }

    if (!createProductInstanceDirectory())
    {
        return false;
    }

    if (handleAugmentProductUuidFile() && handleAugmentProductInstanceUuidFile())
    {
        return true;
    }

    return false;
}

bool EmptyProductInstanceCreator::arePropertiesValid()
{
    if (m_productDirectory.isEmpty() || m_product == nullptr)
    {
        return false;
    }

    return true;
}

bool EmptyProductInstanceCreator::createProductInstanceDirectory()
{
    if (m_productInstanceId.isNull())
    {
        m_productInstanceId = QUuid::createUuid();
    }
    QString productInstanceDirectoryName(m_productInstanceId.toString(QUuid::WithoutBraces) + "-SN-" +
                                         QString::number(m_serialNumber));
    QDir productDirectory(m_productDirectory);

    return productDirectory.mkpath(productInstanceDirectoryName);
}

bool EmptyProductInstanceCreator::handleAugmentProductUuidFile()
{
    QDir productDirectory(m_productDirectory);
    QFile augmentProductUuidFile(m_productDirectory + "/" + productDirectory.dirName() + ".id");
    QFile augmentProductNullUuidFile(m_productDirectory + "/" + QUuid{}.toString(QUuid::WithoutBraces) + ".id");

    if (augmentProductUuidFile.exists() || augmentProductNullUuidFile.exists())
    {
        return true;
    }

    if (!augmentProductUuidFile.open(QIODevice::WriteOnly))
    {
        return false;
    }
    augmentProductUuidFile.close();

    return true;
}

bool EmptyProductInstanceCreator::handleAugmentProductInstanceUuidFile()
{
    QString productInstanceDirectoryName(m_productInstanceId.toString(QUuid::WithoutBraces) + "-SN-" +
                                         QString::number(m_serialNumber));
    QFile augmentProductInstanceUuidFile(m_productDirectory + "/" + productInstanceDirectoryName + "/" +
                                         m_productInstanceId.toString(QUuid::WithoutBraces) + ".id");
    if (!augmentProductInstanceUuidFile.open(QIODevice::WriteOnly))
    {
        return false;
    }
    augmentProductInstanceUuidFile.close();
    return true;
}

EmptyProductInstanceCreator::~EmptyProductInstanceCreator()
{
}

}