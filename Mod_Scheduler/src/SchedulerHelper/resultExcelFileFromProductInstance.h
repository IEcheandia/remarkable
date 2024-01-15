#pragma once
#include <QObject>
#include <QString>

#include "../../Mod_Storage/src/productModel.h"
#include "../../Mod_Storage/src/productMetaData.h"
#include "../../Mod_Storage/src/resultsExporter.h"

namespace precitec
{

namespace storage
{
class ExtendedProductInfoHelper;
}

namespace scheduler
{

class ResultExcelFileFromProductInstance : public QObject
{
    Q_OBJECT
public:
    ResultExcelFileFromProductInstance(QObject* parent = nullptr);
    virtual ~ResultExcelFileFromProductInstance() override;

    void setResultDirectory(const QString &directory);
    void setResultProductInstanceDirectory(const QString &productInstanceDirectory);
    void setProductStorageDirectory(const QString &productStorageDirectory);

    QString resultFileName() const { return m_resultFileName; };

    precitec::storage::ExtendedProductInfoHelper *extendedProductHelper()
    {
        return m_extendedProductHelper;
    }

    void createResultExcelFile();
Q_SIGNALS:
    void resultFileIsCreated();
    void failed();
private:
    void setResultFileName(const QString &resultFileName);
    void checkResultFile();

    QString m_resultDirectory;
    QString m_productInstanceDirectory;
    QString m_productStorageDirectory;

    precitec::storage::ProductModel *m_productModel = nullptr;
    precitec::storage::ProductMetaData m_productMetaData;
    precitec::storage::ResultsExporter *m_resultsExporter = nullptr;
    precitec::storage::ExtendedProductInfoHelper *m_extendedProductHelper;

    QString m_resultFileName;
};

}
}
