#pragma once

#include <QObject>
#include <QUuid>

namespace precitec
{

namespace storage
{
class Product;
}

namespace gui
{
/**
 * The class transfers a content of the source product instance to a newly-created target product instance.
 * Main assumption: seam structure of the source product (seams and seam series)
 * should be the same as the target product otherwise transfer occurs only for a bijection sub-set of seams.
 * The new target product instance has the same serial number as the source product instance.
 **/
class ProductInstanceTransferHandler : public QObject
{
    Q_OBJECT
public:
    explicit ProductInstanceTransferHandler(QObject *parent = nullptr);

    void setDirectory(const QString &directory);
    void setSourceProductInstanceId(const QUuid &id);
    void setSourceSerialNumber(quint32 serialNumber);
    void setSourceProduct(precitec::storage::Product *product);
    void setTargetProduct(precitec::storage::Product *product);

    bool transfer();

    QUuid targetProductInstanceId() const
    {
        return m_targetProductInstanceId;
    }
    QString targetFullProductInstanceDirectory() const;
    ~ProductInstanceTransferHandler() override;

private:
    static QString productDirectoryName(precitec::storage::Product *product);
    static QString productInstanceDirectoryName(const QUuid &productId, const quint32 productSerialNumber);
    QString productDirectory(precitec::storage::Product *product) const;
    QString productInstanceDirectory(precitec::storage::Product *product,
                                        const QUuid &productId,
                                        const quint32 productSerialNumber) const;

    bool arePropertiesInitialized();
    bool doDirectoriesExist();
    bool createEmptyTargetProductInstance();
    void conductTransfer();

    QString m_directory;
    precitec::storage::Product *m_sourceProduct = nullptr;
    precitec::storage::Product *m_targetProduct = nullptr;
    quint32 m_sourceSerialNumber = 0;

    QUuid m_sourceProductInstanceId;
    QUuid m_targetProductInstanceId;

    QString m_targetProductInstanceDirectory;

    QMetaObject::Connection m_sourceProductDestroyedConnection;
    QMetaObject::Connection m_targetProductDestroyedConnection;
};

}
}
