#pragma once

#include <QObject>
#include <QUuid>

#include <vector>

namespace precitec
{

namespace storage
{
class ProductInstanceModel;
class Product;
}

namespace gui
{

class CheckedFilterModel;
class ProductInstanceTransferHandler;

class ProductInstancesTransferController : public QObject
{
    Q_OBJECT
    /**
     * The model includes all checked product instances from sourceProductInstanceModel;
     * Default is @c null.
     **/
    Q_PROPERTY(precitec::gui::CheckedFilterModel *indexFilterModel READ indexFilterModel WRITE setIndexFilterModel NOTIFY indexFilterModelChanged)
    /**
     * The product for which we are going to transferAll checked product instances;
     * Default is @c null.
     **/
    Q_PROPERTY(precitec::storage::Product *targetProduct READ targetProduct WRITE setTargetProduct NOTIFY targetProductChanged)
    /**
     * Whether the ProductInstanceTransferController is currently transferring
     **/
    Q_PROPERTY(bool transferInProgress READ transferInProgress WRITE setTransferInProgress NOTIFY transferInProgressChanged)

public:
    ProductInstancesTransferController(QObject *parent = nullptr);
    ~ProductInstancesTransferController() override;

    precitec::gui::CheckedFilterModel *indexFilterModel() const
    {
        return m_indexFilterModel;
    }
    precitec::storage::Product *targetProduct() const
    {
        return m_targetProduct;
    }

    bool transferInProgress() const
    {
        return m_transferInProgress;
    }

    void setIndexFilterModel(precitec::gui::CheckedFilterModel *model);
    void setTargetProduct(precitec::storage::Product *product);
    void setMonitoring(bool toUse);

    Q_INVOKABLE bool transfer();

    void setTransferInProgress(bool transferInProgress);

    std::vector<QString> targetProductInstanceDirectories() const
    {
        return m_targetProductInstanceDirectories;
    };

Q_SIGNALS:
    void indexFilterModelChanged();
    void targetProductChanged();

    void transferInProgressChanged();

private:
    void setSourceProduct(precitec::storage::Product *product);
    void setSourceProductInstanceModel(precitec::storage::ProductInstanceModel *model);

    bool arePublicPropertiesInitialised();
    bool checkAndInitializePrivateProperties();

    void transferAllConcurrently();
    void transferAll();
    void transfer(quint32 indexFilterModelIndex);
    void runProductInstanceCacheEditor();

    precitec::storage::ProductInstanceModel *m_sourceProductInstanceModel = nullptr;
    precitec::gui::CheckedFilterModel *m_indexFilterModel = nullptr;

    precitec::storage::Product *m_sourceProduct = nullptr;
    precitec::storage::Product *m_targetProduct = nullptr;

    std::vector<QString> m_targetProductInstanceDirectories;

    QMetaObject::Connection m_sourceProductInstanceModelDestroyedConnection;
    QMetaObject::Connection m_indexFilterModelDestroyedConnection;

    QMetaObject::Connection m_targetProductDestroyedConnection;
    QMetaObject::Connection m_sourceProductDestroyedConnection;

    bool m_transferInProgress = false;
    bool m_usingCacheMonitoring = true;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ProductInstancesTransferController *)