#pragma once
#include <QObject>
#include <QUuid>
#include <QString>

namespace precitec
{

namespace storage
{
class Product;
}

namespace gui
{
class EmptyProductInstanceCreator : public QObject
{
    Q_OBJECT
public:
    explicit EmptyProductInstanceCreator(QObject *parent = nullptr);
    /**
     * set the directory in which product instance should be created.
     * Default is an empty, not valid directory path.
     **/
    void setProductDirectory(const QString &directory);
    void setProduct(precitec::storage::Product *product);
    /**
     * The serial number for which an empty product instance should be created.
     * Default is zero.
     **/
    void setSerialNumber(quint32 serialNumber);
    /**
     * The product instance id.
     * Default is zero Uuid.
     * Can be specified or created randomly during create().
     **/
    void setProductInstanceId(const QUuid &id);

    bool create();

    QString productInstanceDirectoryPath() const
    {
        return m_productDirectory;
    }
    precitec::storage::Product *product() const
    {
        return m_product;
    }
    quint32 serialNumber()
    {
        return m_serialNumber;
    }
    QUuid productInstanceId()
    {
        return m_productInstanceId;
    }

    ~EmptyProductInstanceCreator() override;

private:
    bool arePropertiesValid();
    bool createProductInstanceDirectory();
    bool handleAugmentProductUuidFile();
    bool handleAugmentProductInstanceUuidFile();

    QString m_productDirectory;
    precitec::storage::Product *m_product = nullptr;
    quint32 m_serialNumber = 0;
    QUuid m_productInstanceId;

    QMetaObject::Connection m_productDestroyedConnection;
};

}
}